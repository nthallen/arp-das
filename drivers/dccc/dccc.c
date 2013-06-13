/*
 * Discrete command card controller program.
 * $Log$
 * Revision 1.11  2012/03/23 01:01:33  ntallen
 * Add verbosity
 *
 * Revision 1.10  2012/03/21 16:08:06  ntallen
 * Detab
 *
 * Revision 1.9  2012/01/23 16:34:36  ntallen
 * A little more verbosity
 *
 * Revision 1.8  2012/01/19 19:25:07  ntallen
 * A little verbosity in error case.
 *
 * Revision 1.7  2011/02/18 19:41:31  ntallen
 * Reworking of interface preparatory to switching to resmgr
 *
 * Revision 1.6  2010/09/10 13:07:51  ntallen
 * Minor edit and relink for libsubbus.so.1
 *
 * Revision 1.5  2008/09/04 21:25:29  ntallen
 * Added explicit delay, just in case.
 *
 * Revision 1.4  2008/08/24 15:20:19  ntallen
 * Compile clean
 *
 * Revision 1.3  2008/08/24 15:14:30  ntallen
 * Restructure to handle strobe commands without separate scdc app
 *
 * Revision 1.2  2008/08/23 16:19:21  ntallen
 * Compiling
 *
 * Revision 1.1  2008/08/23 02:45:40  ntallen
 * QNX6 mods
 *
 * Modified by Eil July 1991 for QNX.
 * Ported to QNX 4 by Eil 4/15/92.
 * Ported to QNX 6 by Nort 8/23/08.
 */

/* header files */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <hw/inout.h>
#include "subbus.h"
#include "disc_cmd.h"
#include "nortlib.h"
#include "oui.h"
#include "nl_assert.h"
#include "tm.h"
#include "disc_cmd.h"
static char rcsid[] = "$Id$";

/* functions */
void init_cards(void);
void set_line(int port, int mask), reset_line(int port, int mask);
void sel_line(int port, int mask, int value);
void read_commands(void);
int get_type(char *buf, int *type), get_line(FILE *fp, char *buf);
static void process_pcmd( cmd_t *pcmd );
static void execute_pcmd( cmd_t *pcmd, int clr_strobe );
int DCCC_Done = 0;

/* global variables */
struct cfgcmd {
  unsigned int address;
  unsigned int data;
} *cfg_cmds;

struct port {
  int sub_addr;
  int defalt;
  int value;
} *ports;

struct cmd {
  int type;
  int port;
  int mask;
  int value;
} *cmds;

int init_fail = 0;
static int sb_syscon = 0;
static int use_cmdstrobe = 0;
static char cmdfile[FILENAME_MAX] = "dccc_cmd.txt";
static int n_cfgcmds = 0, n_ports = 0, n_cmds = 0;


void dccc_init_options( int argc, char **argv ) {
  int i;
  
  /* process args */
  opterr = 0;
  optind = OPTIND_RESET;
  do {
    i = getopt(argc,argv,opt_string);
    switch (i) {
      case 'f': strncpy(cmdfile, optarg, FILENAME_MAX-1); break;
      case 'i': init_fail = 1; break;
      case '?': nl_error(4,"Invalid option -%c",optopt);
      default:  break;
    }
  } while (i!=-1);
}

/**
  * Syntax for commands will be:
  ^$ Quit
  ^D\d+ Discrete command (STRB or UNSTRB)
  ^S\d+=\d+ SET or SELECT
  ^M\d+[,\d+]*
  ^N\d+=\d+[,\d+=\d+]*
 */
 

static void skip_space( char *buf, int *idx ) {
  int i = *idx;
  while ( isspace(buf[i]) ) ++i;
  *idx = i;
}

/* return non-zero on error */
static int readunum( char *buf, int *idx, unsigned int *val ) {
  int i = *idx;
  unsigned int n;
  skip_space( buf, &i );
  if ( buf[i] == '0' && tolower(buf[i+1]) == 'x' && isxdigit(buf[i+2]) ) {
    i += 2;
    n = 0;
    while (isxdigit(buf[i])) {
      n = (n<<4) + (buf[i] >= 'A') ? buf[i] - 'A' + 10 : buf[i] - '0';
      ++i;
    }
  } else if ( isdigit(buf[i]) ) {
    n = 0;
    while ( isdigit(buf[i]) ) {
      n = (n*10) + buf[i++] - '0';
    }
  } else {
    nl_error( 2, "Expected number" );
    return 1;
  }
  *val = n;
  *idx = i;
  return 0;
}

void parse_cmd(char *tbuf, int nb, cmd_t *pcmd ) {
  nl_assert( nb >= 0 );
  if ( nb == 0 ) {
    pcmd->cmd_type = 'Q';
  } else {
    int i = 0;
    int need_vals = 0;
    int multi = 0;
    int cmd_variety;
    pcmd->n_cmds = 0;
    nl_assert( nb <= DCCC_MAX_CMD_BUF );
    tbuf[nb] = '\0';
    nl_error(MSG_DEBUG, "Recd '%s'", tbuf);
    skip_space( tbuf, &i );
    switch ( tbuf[i] ) {
      case 'D': break;
      case 'S': need_vals = 1; break;
      case 'M': multi = 1; break;
      case 'N': multi = 1; need_vals = 1; break;
      default:
        nl_error( 2, "Invalid command type received" );
        return;
    }
    pcmd->cmd_type = tbuf[i++];
    for (;;) {
      unsigned int val;
      if ( readunum(tbuf, &i, &val) ) {
        nl_error( 2, "n_cmds=%d nb=%d cmd='%s'",
          pcmd->n_cmds, nb, tbuf );
        return;
      }
      pcmd->cmds[pcmd->n_cmds].cmd = val;
      if ( val > n_cmds ) {
        nl_error( 2, "Invalid command number: %d", val );
        return;
      }
      if ( pcmd->n_cmds == 0 ) {
        cmd_variety = cmds[val].type;
        switch (cmd_variety) {
          case STEP:
          case STRB:
          case UNSTRB:
            if ( need_vals ) {
              nl_error(2, "Command does not match header");
              return;
            }
            break;
          case SET:
          case SELECT:
            if ( ! need_vals ) {
              nl_error(2, "Command does not match header");
              return;
            }
            break;
        }
      } else {
        if ( cmds[val].type != cmd_variety ) {
          nl_error( 2, "Mismatched command in multi" );
          return;
        }
      }
      if ( need_vals ) {
        skip_space( tbuf, &i );
        if ( tbuf[i++] != '=' ) {
          nl_error( 2, "Expected '=' for value" );
          return;
        }
        readunum( tbuf, &i, &val );
        pcmd->cmds[pcmd->n_cmds].value = val;
      }
      ++pcmd->n_cmds;
      skip_space( tbuf, &i );
      if ( tbuf[i] == '\0' ) break;
      else if ( tbuf[i] == ',' ) {
        if ( multi ) {
          i++;
          if ( pcmd->n_cmds >= MAX_CMDS ) {
            nl_error( 2, "Too many commands in multi-command" );
            return;
          }
        } else {
          nl_error(2,"Multiple commands listed for single-command syntax");
          return;
        }
      } else {
        nl_error( 2, "Syntax error" );
        return;
      }
    }
  }
  process_pcmd(pcmd);
}

int main(int argc, char **argv) {

  oui_init_options( argc, argv );
  nl_error( 0, "Startup" );


  /* subbus */
  if (subbus_subfunction == SB_SYSCON || 
      subbus_subfunction == SB_SYSCON104 ||
      subbus_subfunction == SB_SYSCONDACS) sb_syscon = 1;
  if (subbus_features & SBF_CMDSTROBE) use_cmdstrobe = 1;
  else if (sb_syscon) 
    nl_error(MSG_WARN,"Out of date resident subbus library: please upgrade");

  read_commands();
  init_cards();
  operate();
  nl_error( -1, "Shutdown" );
  return 0;
}

static void process_pcmd(cmd_t *pcmd) {
  int dccc_cmd_type;

  /* check out msg structure */
  if ( pcmd->cmd_type == 'Q') {
    // nl_error( -1, "Shutdown" );
    DCCC_Done = 1;
    return;
  }
  dccc_cmd_type = cmds[pcmd->cmds[0].cmd].type;
  execute_pcmd( pcmd, 0 );
  if (dccc_cmd_type == STRB) {
    /* optionally add a delay here */
    delay(10);
    execute_pcmd( pcmd, 1 );
  }
}

static void execute_pcmd( cmd_t *pcmd, int clr_strobe ) {
  int cmd_idx = pcmd->cmds[0].cmd;
  unsigned short value;
  int dccc_cmd_type = cmds[cmd_idx].type;
  int cmd_ok = 1;
  static int str_cmd;
  int i;
  
   /* reset the strobe if necessary or define str_cmd */
  if (dccc_cmd_type == STRB) {
    if (clr_strobe) {
      if (sb_syscon) {
        if (use_cmdstrobe) set_cmdstrobe(0);
        else out8(0x30E, 2);
      } else reset_line(cmds[n_cmds-1].port,cmds[n_cmds-1].mask);
      if (cmd_idx != str_cmd) nl_error(MSG_WARN, "Bad strobe sequence");
    } else str_cmd = cmd_idx;
  }

  for ( i = 0; cmd_ok && i < pcmd->n_cmds; i++ ) {
    cmd_idx = pcmd->cmds[i].cmd;
    value = pcmd->cmds[i].value;

    switch (dccc_cmd_type) {
    case STRB:
      if (clr_strobe) {
        nl_error(MSG_DEBUG, "STRB reset: PORT %03X mask %04X command index %d",
          ports[cmds[cmd_idx].port].sub_addr, cmds[cmd_idx].mask, cmd_idx);
        reset_line(cmds[cmd_idx].port, cmds[cmd_idx].mask);
      } else {
        nl_error(MSG_DEBUG,"STRB set: port %03X mask %04X command index %d",
          ports[cmds[cmd_idx].port].sub_addr, cmds[cmd_idx].mask,cmd_idx);
        set_line(cmds[cmd_idx].port, cmds[cmd_idx].mask);
      }
      break;
    case STEP:
      set_line(cmds[cmd_idx].port, cmds[cmd_idx].mask);
      reset_line(cmds[cmd_idx].port, cmds[cmd_idx].mask);
      break;
    case SET:
      nl_error(MSG_DEBUG,
        "SET: PORT %03X, mask %04X, value %04X, command index %d",
        ports[cmds[cmd_idx].port].sub_addr, cmds[cmd_idx].mask,
        value, cmd_idx);
      if (cmds[cmd_idx].mask)
        if (value) set_line(cmds[cmd_idx].port, cmds[cmd_idx].mask);
        else reset_line(cmds[cmd_idx].port, cmds[cmd_idx].mask);
      else set_line(cmds[cmd_idx].port, value);
      break;
    case SELECT:
      // For historical reasons, SELECT uses negative logic
      // in the value arg
      nl_error(MSG_DEBUG,
        "SELECT: PORT %03X, mask %04X, value %04X, command index %d",
        ports[cmds[cmd_idx].port].sub_addr, cmds[cmd_idx].mask,
        value, cmd_idx);
      sel_line(cmds[cmd_idx].port, cmds[cmd_idx].mask, ~value);
      break;
    case UNSTRB:
      value = cmds[cmd_idx].value;
      nl_error(MSG_DEBUG,
        "UNSTRB: PORT %03X, mask %04X, value %04X, command index %d",
        ports[cmds[cmd_idx].port].sub_addr, cmds[cmd_idx].mask,
        value, cmd_idx);
      sel_line(cmds[cmd_idx].port, cmds[cmd_idx].mask, value);
      break;
    case SPARE: cmd_ok = 0;
      nl_error(MSG_WARN,"command %d of type SPARE received", cmd_idx);
      break;
    default: cmd_ok = 0;
      nl_error(MSG_WARN, "command %d: unknown command type %d received",
        cmd_idx, cmds[cmd_idx].type);
    } /* switch */
  }

  if (dccc_cmd_type == STRB) {
    if (!clr_strobe) {
      if (sb_syscon) {
        if (use_cmdstrobe) set_cmdstrobe(1);
        else out8(0x30E, 3);
      } else set_line(cmds[n_cmds-1].port, cmds[n_cmds-1].mask);
    }
  }
}	/*execute_pcmd */


/* functions */

/* return 1 on success, 0 on error */
void init_cards(void) {
  int i;
  for (i = 0; i < n_cfgcmds; i++)
    if (!write_ack(cfg_cmds[i].address, cfg_cmds[i].data))
      nl_error(init_fail ? MSG_EXIT_ABNORM : MSG_WARN,"No ack for configuration command at address %#X",cfg_cmds[i].address);
  for (i = 0; i < n_ports; i++)
    if (!write_ack(ports[i].sub_addr, ports[i].defalt))
      nl_error(init_fail ? MSG_EXIT_ABNORM : MSG_WARN,"No ack at port address %#X",ports[i].sub_addr);
  /* Set Command Strobe Inactive */
  if (sb_syscon) {
    if (use_cmdstrobe) set_cmdstrobe(0);
    else out8(0x30E, 2);
  }
  else reset_line(cmds[n_cmds-1].port,cmds[n_cmds-1].mask);
  /* Enable Commands */
  set_cmdenbl(1);
}

void set_line(int port, int mask) {
  ports[port].value |= mask;
  write_subbus(ports[port].sub_addr, ports[port].value);
}

void old_line(int port) {
  ports[port].value = read_subbus(ports[port].sub_addr);
}

void sel_line(int port, int mask, int value) {
  ports[port].value &= ~mask;
  ports[port].value |= (mask & value);
  write_subbus(ports[port].sub_addr, ports[port].value);
}

void reset_line(int port, int mask) {
  ports[port].value &= ~mask;
  write_subbus(ports[port].sub_addr, ports[port].value);
}

/* No special commands for the time being */

void read_commands(void) {
  FILE *fp;
  char buf[128], *p;
  int i;

  if ((fp = fopen(cmdfile, "r")) == NULL)
    nl_error(MSG_EXIT_ABNORM,"error opening %s",cmdfile);

  if (get_line(fp, buf) || (sscanf(buf, "%d", &n_cfgcmds) != 1))
    nl_error(MSG_EXIT_ABNORM,"error getting number of config commands");

  cfg_cmds = (struct cfgcmd *) malloc(n_cfgcmds * sizeof(struct cfgcmd));

  /* Configuration commands */
  for (i = 0; i < n_cfgcmds; i++)
    if (get_line(fp, buf) ||
        (sscanf(buf, "%x,%x", &cfg_cmds[i].address, &cfg_cmds[i].data) != 2))
      nl_error(MSG_EXIT_ABNORM, "error getting configuration command %d",i);

  if (get_line(fp, buf) || (sscanf(buf, "%d", &n_ports) != 1))
    nl_error(MSG_EXIT_ABNORM, "error getting number of ports");

  ports = (struct port *) malloc(n_ports * sizeof(struct port));

  /* Port information */
  for (i = 0; i < n_ports; i++) {
    if ((get_line(fp, buf))) 
      nl_error(MSG_EXIT_ABNORM, "error getting port %d info",i);
    switch(sscanf(buf, "%x,%x", &ports[i].sub_addr, &ports[i].defalt)) {
    case 2: ports[i].value = 0; break;
    /*** if a defalt value is not scanned, set it and value to hardware ***/
    case 1: old_line(i); ports[i].defalt = ports[i].value; break;
    case 0:
    case EOF: nl_error(MSG_EXIT_ABNORM, "error getting port %d info",i);
    }
  }

  if (get_line(fp, buf) || (sscanf(buf, "%d", &n_cmds) != 1))
    nl_error(MSG_EXIT_ABNORM, "error getting number of commands");

  cmds = (struct cmd *) malloc(n_cmds * sizeof(struct cmd));

  /* Commands */
  for (i = 0; i < n_cmds; i++) {
    int n_args;
    if (get_line(fp,buf) || get_type(buf, &cmds[i].type))
      nl_error(MSG_EXIT_ABNORM, "error getting command %d type", i);
    for (p = &buf[0]; *p != ','; p++);
    n_args = sscanf(p, ",%d,%x,%x", &cmds[i].port, &cmds[i].mask,
      &cmds[i].value);
    if (n_args == 3) {
      if (cmds[i].type != UNSTRB)
        nl_error(MSG_EXIT_ABNORM, "Too many arguments for command %d");
    } else if (n_args == 2) {
      if (cmds[i].type == UNSTRB)
        nl_error(MSG_EXIT_ABNORM, "Missing value for command %d");
    } else {
      nl_error(MSG_EXIT_ABNORM, "error getting command %d info");
    }
  }

  /* dont read special commands */

  fclose(fp);
}
