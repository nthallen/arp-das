/* 
Soldrv.c drives the solenoids from the compiled solenoid format file.
Written April 9, 1987
Modified for more complexities March 24, 1988
Upgraded to Lattice V6.0 April 13, 1990
Modified by Eil July 1991 for QNX.
July 1991: soldrv does not yet accept multiple SOL_STROBES and SOL_DTOA commands.
July 1991: soldrv does not yet send MULTCMDS to scdc.
Ported to QNX 4 by Eil 4/20/92.
Ported to QNX6 by NTA 2/22/11.
*/

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <libgen.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <sys/iomsg.h>
// #include <sys/timers.h>
// #include <sys/types.h>
#include "subbus.h"
// #include "das.h"
// #include "eillib.h"
#include "nortlib.h"
// #include "scdc.h"
// #include "soldrv.h"
#include "sol.h"
#include "codes.h" // in solfmt directory
// #include "da_cache.h"
#include "oui.h"
#include "msg.h"
#include "tm.h"
#include "collect.h"
#include "nl_assert.h"


timer_t timer=0;
struct itimerspec tval, otval;
unsigned int SolStat = 0;

/* defines */
// #define HDR "sol"
// #define OPT_MINE "qd:"
#define ST_NO_MODE 0
#define ST_NEW_MODE 1
#define ST_MODE 2
#define ST_IN_MODE 0
#define ST_WAITS 1
#define CACHE_ADDR 0x1000
#define MAX_SOL_CMD 50

/* function declarations */
static void waitmsg(int, int);
static int read_cmd(void);

/* global variables */
static char rcsid[]="$Id$";
int state;
int mode_request;
int crnt_mode, new_mode;
int Quit = 0;
// unsigned char dct = DCT_SOLDRV_A;
// pid_t *proxy_pids;
// unsigned char reg_which;
unsigned short cacher = CACHE_ADDR;
static int cmd_fd, dccc_fd = -1;
static struct sigevent cmd_event;

static void timer_stop(void) {
  tval.it_value.tv_sec = 0L;
  tval.it_value.tv_nsec = 0L;
  if (timer_settime(timer, 0, &tval, NULL) )
    nl_error(3, "Error %d stopping timer", errno );
}

static void timer_start( unsigned int count) {
  int i, j;

  /* converts back */
  if (count==0) i = 4;
  else i = count/16384;
  j = (count%16384) / 16;
  j = (j*1953125) / 2;
  /* set timer resolution */
  tval.it_value.tv_sec = (long)i;
  tval.it_value.tv_nsec = (long)j;
  tval.it_interval.tv_sec = (long)i;
  tval.it_interval.tv_nsec = (long)j;
  nl_error( -2, "Timer resolution: %d sec, %d nsec", i, j);
  if (timer_settime(timer, 0, &tval, NULL) )
    nl_error(3, "Error %d starting timer", errno );
}
  
static void timer_attach(int coid, int code, int value) {
  struct sigevent event;
  event.sigev_notify = SIGEV_PULSE;
  event.sigev_coid = coid;
  event.sigev_priority = SIGEV_PULSE_PRIO_INHERIT;
  event.sigev_code = code;
  cmd_event.sigev_value.sival_int = value;
  if ( timer_create(CLOCK_REALTIME, &event, &timer) )
    nl_error(3, "Error %d in timer_create", errno );
}

static void timer_detach(void) {
  timer_delete(timer);
}

void my_signalfunction(int sig_number) {
  Quit = 1;
  // timer_detach();
  // exit(0);
}


#define CMD_PULSE_CODE (_PULSE_CODE_MINAVAIL + 0)
#define TIMER_PULSE_CODE (_PULSE_CODE_MINAVAIL + 1)

void soldrv_init_options( int argc, char **argv ) {
  extern int optind, opterr, optopt;
  int i, j, exp_ext;
  char filename[40];

  /* process args */
  opterr = 0;
  optind = OPTIND_RESET;
  do {
    i = getopt(argc,argv,opt_string);
    switch (i) {
      case 'd': cacher = atoh(optarg); break;
      case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
      default: break;
    }
  } while (i!=-1);

  if (optind >= argc)
    msg(MSG_EXIT_ABNORM,"No solenoid file specified on command line");

  for (exp_ext = 0; optind < argc; optind++) {
    if (exp_ext != 0) msg(MSG_EXIT_ABNORM,"Too many input files");
    for (j = 0; argv[optind][j] != '\0'; j++) {
      if (argv[optind][j] == '/') exp_ext = 0;
      else if (argv[optind][j] == '.') exp_ext = j;
      filename[j] = argv[optind][j];
    }
    if (exp_ext == 0) strcpy(filename+j, ".sft");
    else filename[j] = '\0';
    exp_ext = 1;
    i = read_sft(filename);
    if (i==1) msg(MSG_EXIT_ABNORM,"Can't open %s",basename(filename));
    if (i==-1) msg(MSG_EXIT_ABNORM,"Wrong version: %s",basename(filename));
  }
}

static void close_cmd_fd(void) {
  close(cmd_fd);
}

static void open_cmd_fd( int coid, short code, int value, char which_sol ) {
  char cmdbase[12] = "cmd/SoldrvA";
  int old_response = set_response(0);
  cmdbase[10] = which_sol;
  cmd_fd = tm_open_name(tm_dev_name(cmdbase), NULL, O_RDONLY|O_NONBLOCK);
  set_response(old_response);
  if ( cmd_fd < 0 ) {
    nl_error(3, "Unable to open command channel" );
  }

  /* Initialize cmd event */
  cmd_event.sigev_notify = SIGEV_PULSE;
  cmd_event.sigev_coid = coid;
  cmd_event.sigev_priority = SIGEV_PULSE_PRIO_INHERIT;
  cmd_event.sigev_code = code;
  cmd_event.sigev_value.sival_int = value;
}

int main(int argc, char **argv) {

  /* getopt variables */

  /* local variables */
  int i, ip, mode_mode;
  long j;
  unsigned int count;
  send_id SolStat_id;
  int coid, chid;

  oui_init_options( argc, argv );

  /* initialisations */
  mode_request = 0;
  state = ST_NO_MODE;

  signal(SIGQUIT,my_signalfunction);
  signal(SIGINT,my_signalfunction);
  signal(SIGTERM,my_signalfunction);
  
  if (which < 'A' || which > 'J')
    msg(MSG_EXIT_ABNORM,"unknown SOLDRV_PROXY type '%c'", which);

  /* Setup communications:
     Need a channel and connection ID to receive pulses
     Need to open command fd for cmd/soldrvA (etc.) and use
       ionotify to get pulses when commands are ready
     Need to create a timer and configure for getting a pulse
     
     Code used here is very similar to that used in idx64.
  */
  
  /* Initialize channel to receive pulses*/
  chid = ChannelCreate(0);
  if ( chid == -1 )
    nl_error( 3, "Error %d from ChannelCreate()", errno );
  coid = ConnectAttach( ND_LOCAL_NODE, 0, chid, _NTO_SIDE_CHANNEL, 0);
  if ( coid == -1 )
    nl_error( 3, "Error %d from ConnectAttach()", errno );

  /* read commands from command server */
  open_cmd_fd( coid, CMD_PULSE_CODE, 0, which );
 
  /* attach timer */
  timer_attach( coid, TIMER_PULSE_CODE, 0);

  /* look for subbus if necessary */
  if (n_set_points) {
    set_response(NLRSP_QUIET);	/* for Col_send_init() */    		
    for (j=0,i=0;i<n_set_points;i++) {
      if (set_points[i].address==0) {
        if (j++ == 0) { /* Only need to register once */
          char stat_name[9] = "SolStatX";
          stat_name[7] = which;
          SolStat_id = Col_send_init(stat_name, &SolStat, sizeof(SolStat), 0);
        }
      } else if (set_points[i].address >= cacher) {
        j++;
      }
    }
    // if (seteuid(0)==-1) msg(MSG_EXIT_ABNORM,"Can't set euid to root");
    // All set points go through subbusd now
    if (!load_subbus())
      msg(MSG_EXIT_ABNORM,"Requires resident Subbus Library");
  }

  /* Look for DCCC */
  if (n_solenoids) {
    dccc_fd = open(tm_dev_name("dccc"), O_WRONLY);
    if (dccc_fd < 0)
      nl_error(3, "Cannot locate dccc" );
  }

  /* set up proxies */
  if (n_proxies) 
    nl_error(4, "Proxies not implemented yet" );

  /* arm command input */
  read_cmd();

  /* program code */
  while (!Quit) {
    switch (state) {
      case ST_NO_MODE:
        /* wait for a msg from anyone except timer */
        msg(MSG,"No Mode");
        waitmsg(0, chid);
        break;
      case ST_NEW_MODE:
        crnt_mode = new_mode;
        ip = mode_indices[new_mode];
        state = ST_MODE;
        mode_mode = ST_IN_MODE;
        mode_request = 0;
        msg(MSG,"Mode %d",crnt_mode);
      case ST_MODE:
      default:
        /* In a Mode */
        switch (mode_mode) {
          case ST_IN_MODE:
          default:
            switch (mode_code[ip]) {
              case SOL_STROBES:
              case SOL_MULT_STROBES:
                /* send discreet commands to DCCC */
                { int nbs, nbw;
                  char *s;
                  ++ip;
                  nl_assert(mode_code[ip] < str_tbl_size);
                  s = str_tbl[mode_code[ip]];
                  nbs = strlen(s);
                  nbw = write(dccc_fd, s, nbs);
                  if ( nbw != nbs )
                    nl_error(3, "Error %d sending to dccc", errno);
                  ++ip;
                }
                break;
              case SOL_WAIT:
                msg(MSG_DEBUG,"Step %d wait", ip);
                waitmsg(1, chid);
                msg(MSG_DEBUG,"Step %d", ip);
                ip++;
                break;
              case SOL_SET_TIME:
                /* resolution and set timer going */
                count = mode_code[ip+1] + (mode_code[ip+2] << 8);
                timer_start(count);
                ip += 3;
                break;
              case SOL_GOTO:
                ip = mode_code[ip+1] + (mode_code[ip+2] << 8);
                break;
              case SOL_END_MODE:
                state = ST_NO_MODE;
                timer_stop();
                msg(MSG,"Installed");
                break;
              case SOL_WAITS:
                count = mode_code[ip+1];
                mode_mode = ST_WAITS;
                ip += 2;
                break;
              case SOL_SELECT:
                timer_stop();
                if (mode_indices[mode_code[ip+1]]!=-1) {
                  state = ST_NEW_MODE;
                  new_mode = mode_code[ip+1];
                } else {
                  if (mode_code[ip+1]!=0)
                    msg(MSG_WARN,"can't select undefined mode %d",mode_code[ip+1]);
                  state = ST_NO_MODE;
                }
                break;
              case SOL_MSWOK:
                if (mode_request != 0) {
                  timer_stop();
                  state = ST_NEW_MODE;
                } else ip++;
                break;
              case SOL_DTOA:
                if (set_points[mode_code[ip+1]].address == 0) {
                  msg(MSG_DBG(1),"Setting shared memory status to %d",
                      set_points[mode_code[ip+1]].value);
                  SolStat = set_points[mode_code[ip+1]].value;
                  Col_send(SolStat_id);
                } else
                  if (set_points[mode_code[ip+1]].address >= cacher) {
                    msg(MSG_DBG(1),"Caching %d to address %d",
                      set_points[mode_code[ip+1]].value,
                      set_points[mode_code[ip+1]].address);
                    cache_write(set_points[mode_code[ip+1]].address,
                                set_points[mode_code[ip+1]].value);
                  } else {
                    msg(MSG_DBG(1),"Writing %d to address %d",
                      set_points[mode_code[ip+1]].value,
                      set_points[mode_code[ip+1]].address);
                    sbwr(set_points[mode_code[ip+1]].address,
                      set_points[mode_code[ip+1]].value);
                  }
                ip += 2;
                break;
              case SOL_PROXY:
                //if (proxy_pids[mode_code[++ip]])
                //  if (Trigger(proxy_pids[mode_code[ip]])==-1)
                //    msg(MSG_WARN,"Can't trigger proxy %d",proxy_pids[ip]);
                // ip++;
                break;
            }			/* switch */
            break;
          case ST_WAITS:
            msg(MSG_DEBUG,"Step %d waits count %d", ip, count);
            waitmsg(1, chid);
            msg(MSG_DEBUG,"Step %d", ip);
            if (--count <= 0) mode_mode = ST_IN_MODE;
            break;
        }				/* switch */
        break;
    }				/* switch */
  }				/* for(;;) */
  msg(0, "Terminating");
  close_cmd_fd();
  timer_detach();
  if (dccc_fd >= 0) close(dccc_fd);
  msg(0, "Terminated");
  return 0;
}				/* main */

/**
  * @return 0 anytime anything significant happens
  * Command Syntax:
  *   Empty command is quit
  *  S%d\n Select Mode n
  *  
  * I may just eliminate the Set and Reset Proxy commands
  * and just have soldrv send directly to command server.
  */
  
static int parse_command( char *buf, int nb ) {
  int val = 0;
  char *s = buf;
  
  buf[nb] = '\0';
  switch (*s) {
    case 'S': /* Select Mode */
      if (isdigit(*++s)) {
        while (isdigit(*s)) {
          val = val*10 + *s++ - '0';
        }
        if ( *s == '\n' || *s == '\0' ) {
          /* command syntax is OK. Now does it require immediate action? */
          if (val == 0) {
            timer_stop();
            // invite=0; ### check this out
            if (mode_indices[0] == -1) state = ST_NO_MODE;
            else {
              state = ST_NEW_MODE;
              new_mode = 0;
            }
            return 0; // Yes, act on it.
          } else {
            new_mode = val;
            if (state == ST_NO_MODE) {
              state = ST_NEW_MODE;
              return 0;
            } else {
              mode_request = 1;
              msg(MSG,"Mode %d: %d Pending",crnt_mode, new_mode);
              return 1;
            }
          }
        }
      }
      break;
    default: break;
  }
  msg(2, "Invalid command: '%s'", buf);
  return 1;
}

/**
  * @return 0 anytime anything significant happens
  */
static int read_cmd(void) {
  char buf[MAX_SOL_CMD];
  int errs = 0;
  
  for (;;) {
    int rv;
    rv = read(cmd_fd, buf, MAX_SOL_CMD-1);
    if ( rv > 0 ) {
      buf[rv] = '\0';
      errs = parse_command( buf, rv );
    } else if ( rv == 0 ) {
      Quit = 1;
      return 0;
    } else if ( rv == -1 ) {
      if ( errno != EAGAIN )
        nl_error(3, "Received error %d from read() in check_command()", errno );
    } else nl_error(4, "Bad return value [%d] from read in check_command", rv );
    rv = ionotify(cmd_fd, _NOTIFY_ACTION_POLLARM,
        _NOTIFY_COND_INPUT, &cmd_event);
    if (rv == -1) nl_error( 3,
      "Received error % from iontify in check_command()", errno );
    if (rv == 0) break;
  }
  return errs;
}

static void waitmsg ( int timer_expected, int chid ) {
  int rv;
  struct _pulse pulse;

  do {
    if ( MsgReceivePulse(chid, &pulse, sizeof(pulse), NULL) == -1 ) {
      if ( errno == EINTR) {
	if ( !Quit )
	  msg(MSG_WARN, "Receive EINTR, but Quit == 0");
      } else {
	msg(MSG_WARN,"error %d recieving pulses", errno);
      }
      rv = 1;
    } else if (pulse.code == CMD_PULSE_CODE) {
      rv = read_cmd(); /* read_cmd() must return 0 when we should exit */
    } else if ( pulse.code == TIMER_PULSE_CODE ) {
      rv = timer_expected ? 0 : 1;
      if (!timer_expected) msg(MSG_WARN, "Unexpected timer pulse");
    } else msg(MSG_EXIT_ABNORM,"Unexpected pulse code: %d", pulse.code);
  } while (rv == 1 && Quit == 0);
}
