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
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/neutrino.h>
// #include <sys/types.h>
#include "subbus.h"
// #include "das.h"
// #include "eillib.h"
#include "nortlib.h"
// #include "scdc.h"
#include "soldrv.h"
#include "sol.h"
// #include "codes.h"
// #include "da_cache.h"
#include "oui.h"

  // #include <sys/timers.h>

  timer_t timer=0;
  struct itimercb tcb;
  struct itimerspec tval, otval;
  unsigned int SolStat = 0;

  #define DOWN_TIMER { \
      tval.it_value.tv_sec = 0L; \
      tval.it_value.tv_nsec = 0L; \
      if (reltimer(timer, &tval, 0) == -1) \
    msg(MSG_EXIT_ABNORM, "Error in reltimer"); \
      while (Creceive(timer_proxy,0,0)!=-1); \
      errno=0; \
  }
  #define UP_TIMER \
      if (reltimer(timer, &tval, 0) == -1) \
    msg(MSG_EXIT_ABNORM, "Error in reltimer")
  #define RESOLUTION_TIMER { \
      /* converts back */ \
      if (count==0) i = 4; \
      else i = count/16384; \
      j = (count%16384) / 16; \
    j = (j*1953125) / 2; \
      /* set timer resolution */ \
      tval.it_value.tv_sec = (long)i; \
      tval.it_value.tv_nsec = (long)j; \
      tval.it_interval.tv_sec = (long)i; \
      tval.it_interval.tv_nsec = (long)j; \
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

/* defines */
#define HDR "sol"
#define OPT_MINE "qd:"
#define ST_NO_MODE 0
#define ST_NEW_MODE 1
#define ST_MODE 2
#define ST_IN_MODE 0
#define ST_WAITS 1
#define CACHE_ADDR 0x1000

/* function declarations */
void waitmsg(int);
void my_signalfunction(int sig_number) {
    DETACH_TIMER;
    exit(0);
}

/* global variables */
static char rcsid[]="$Id$";
int state;
int mode_request;
int crnt_mode, new_mode;
// unsigned char dct = DCT_SOLDRV_A;
// pid_t *proxy_pids;
unsigned char reg_which;
unsigned short cacher = CACHE_ADDR;
static int cmd_fd;
static struct sigevent cmd_event;

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

static void open_cmd_fd( int coid, short code, int value ) {
  int old_response = set_response(0);
  char *cmddev = tm_dev_name("cmd/idx64");
  close_cmd_fd();
  cmd_fd = tm_open_name(cmddev, NULL, O_RDONLY|O_NONBLOCK);
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

static void close_cmd_fd(void) {
  close(cmd_fd);
}

static void open_cmd_fd( int coid, short code, int value ) {
  int old_response = set_response(0);
  char *cmddev = tm_dev_name("cmd/idx64");
  close_cmd_fd();
  cmd_fd = tm_open_name(cmddev, NULL, O_RDONLY|O_NONBLOCK);
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


void main(int argc, char **argv) {

  /* getopt variables */

  /* local variables */
  char name[FILENAME_MAX+1];
  int i, exp_ext, ip, mode_mode;
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
  open_cmd_fd( coid, CMD_PULSE_CODE, 0 );
 
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
    if (n_set_points > j) {	
      // if (seteuid(0)==-1) msg(MSG_EXIT_ABNORM,"Can't set euid to root");
      if (!load_subbus())
        msg(MSG_EXIT_ABNORM,"Requires resident Subbus Library");
    }
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

  /* program code */
  for (;;) {
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
                RESOLUTION_TIMER;
                /* up timer */
                UP_TIMER;
                ip += 3;
                break;
              case SOL_GOTO:
                ip = mode_code[ip+1] + (mode_code[ip+2] << 8);
                break;
              case SOL_END_MODE:
                state = ST_NO_MODE;
                DOWN_TIMER;
                msg(MSG,"Installed");
                break;
              case SOL_WAITS:
                count = mode_code[ip+1];
                mode_mode = ST_WAITS;
                ip += 2;
                break;
              case SOL_SELECT:
                DOWN_TIMER;
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
                  DOWN_TIMER;
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
                    write_subbus(0,set_points[mode_code[ip+1]].address,
                      set_points[mode_code[ip+1]].value);
                  }
                ip += 2;
                break;
              case SOL_PROXY:
                if (proxy_pids[mode_code[++ip]])
                  if (Trigger(proxy_pids[mode_code[ip]])==-1)
                    msg(MSG_WARN,"Can't trigger proxy %d",proxy_pids[ip]);
                ip++;
                break;
            }			/* switch */
            break;
          case ST_WAITS:
            msg(MSG_DEBUG,"Step %d waits", ip);
            waitmsg(1, chid);
            msg(MSG_DEBUG,"Step %d", ip);
            if (--count <= 0) mode_mode = ST_IN_MODE;
        }				/* switch */
    }				/* switch */
  }				/* for(;;) */

}				/* main */

/*
Wait for msg from 'invite'. Typically from cmdctrl, timer or processes
(re-)setting proxies.
Waiting for msg from invite=0 and recieving timer msg is an error.
*/

reply_type reply_back(pid_t id, reply_type r, void *s);

/* read_cmd() returns 1 on any errors, 0 otherwise */
static int read_cmd(void) {
  char cmd[MAX_SOL_CMD];
  int errs = 0;
  
  for (;;) {
    int rv;
    rv = read(cmd_fd, buf, MAX_SOL_CMD-1);
    if ( rv > 0 ) {
      buf[rv] = '\0';
      errs = parse_command( buf, rv );
    } else if ( rv == 0 ) return 1; // quit command
    else if ( rv == -1 ) {
      if ( errno != EAGAIN )
        nl_error(3, "Received error %d from read() in check_command()", errno );
    } else nl_error(4, "Bad return value [%d] from read in check_command", rv );
    rv = ionotify(cmd_fd, _NOTIFY_ACTION_POLLARM,
        _NOTIFY_COND_INPUT, &cmd_event);
    if (rv == -1) nl_error( 3, "Received error % from iontify in check_command()", errno );
    if (rv == 0) break;
  }
  return errs;
}
  switch (sol_msg.msg_hdr) {
    case DASCMD:
      if (sol_msg.u.dasc_msg.type != dct ||
        sol_msg.u.dasc_msg.val >= n_modes ||
          (mode_indices[sol_msg.u.dasc_msg.val] == -1
            && sol_msg.u.dasc_msg.val != 0) ) {
        /* bad received msg */
        msg(MSG_WARN,"bad received DASCmd msg type %d val %d",sol_msg.u.dasc_msg.type,sol_msg.u.dasc_msg.val);
        replymsg = reply_back(recv_pid,DAS_UNKN,0);
      }
      else {
        /* good received msg */
        replymsg = reply_back(recv_pid,DAS_OK,0);
        if (sol_msg.u.dasc_msg.val == 0) {
          DOWN_TIMER;
          invite=0;
          if (mode_indices[0] == -1) state = ST_NO_MODE;
          else {
            state = ST_NEW_MODE;
            new_mode = 0;
          }
        } else {
          new_mode = sol_msg.u.dasc_msg.val;
          if (state == ST_NO_MODE) state = ST_NEW_MODE;
          else {
            mode_request = 1;
            msg(MSG,"Mode %d: %d Pending",crnt_mode, new_mode);
          }
        }
      }
      break;
    default:
      if (sol_msg.msg_hdr==reg_which)
        switch (sol_msg.u.proxy_reg.set_or_reset) {
          case SOL_SET_PROXY:
            for (i = 0; i < n_proxies; i++) {
              if (proxy_ids[i] == sol_msg.u.proxy_reg.proxy_id) {
                if (proxy_pids[i] == 0) {
                  proxy_pids[i] = sol_msg.u.proxy_reg.proxy_pid;
                  replymsg = reply_back(recv_pid, DAS_OK, 0);
                } else {
                  msg(MSG_WARN,"proxy id %d already in use to proxy %d",proxy_ids[i],proxy_pids[i]);
                  replymsg = reply_back(recv_pid, DAS_BUSY, 0);
                }
                break;
              }
            }
            if (i>=n_proxies) {
              msg(MSG_WARN,"no such proxy id %d",sol_msg.u.proxy_reg.proxy_id);
              replymsg = reply_back(recv_pid,DAS_UNKN, 0);
            }
            break;
          case SOL_RESET_PROXY:
            for (i = 0; i < n_proxies; i++) {
              if (proxy_ids[i] == sol_msg.u.proxy_reg.proxy_id) {
                if (proxy_pids[i] == 0) {
                  msg(MSG_WARN,"proxy id %d not in use",sol_msg.u.proxy_reg.proxy_id);
                  replymsg = reply_back(recv_pid,DAS_UNKN, &sol_msg);
                }
                else {
                  sol_msg.u.proxy_reg.proxy_pid = proxy_pids[i];
                  proxy_pids[i] = 0;
                  replymsg = reply_back(recv_pid, DAS_OK, &sol_msg);
                }
                break;
              }
            }
            if (i>=n_proxies) {
              msg(MSG_WARN,"no such proxy id %d",sol_msg.u.proxy_reg.proxy_id);
              replymsg = reply_back(recv_pid,DAS_UNKN,&sol_msg);
            }
            break;
        } /* switch */
        else {
          msg(MSG_WARN,"Unknown msg received with header %d",sol_msg.msg_hdr);
          replymsg = reply_back(recv_pid,DAS_UNKN,0);
        }
  } /* switch */
}

void waitmsg ( int timer_expected, int chid ) {
  int i, rv;
  struct _pulse pulse;

  do {
    if ( MsgReceivePulse(chid, &pulse, sizeof(pulse), NULL) == -1 ) {
      msg(MSG_WARN,"error %d recieving pulses", errno);
      rv = 1;
    } else if (pulse.code == CMD_PULSE_CODE) {
      rv = read_cmd(); /* read_cmd() must return 0 when we should exit */
    } else if ( pulse.code == TIMER_PULSE_CODE ) {
      rv = timer_expected ? 1 : 0;
      if (!timer_expected) msg(MSG_WARN, "Unexpected timer pulse");
    } else msg(MSG_EXIT_ABNORM,"Unexpected pulse code: %d", pulse.code);
  } while (rv == 1);
}

/* reply routine */
reply_type reply_back(pid_t id, reply_type rep, void *s) {
  char str[25]="";
  switch(rep) {
      case DAS_UNKN: strcpy(str,"UNKNOWN"); break;
      case DAS_BUSY: strcpy(str,"BUSY");	break;
      case DAS_OK: break;
      default: sprintf(str,"undefined reply code %u",rep);
  }
  if (strlen(str)) msg(MSG_WARN,"replying %s to task %d",str,id);

  /* this is for norton's convenience */
  if (s) {
    memcpy(s,&rep,sizeof(reply_type));
    if (Reply(id, s, sizeof(reply_type) + sizeof(proxy_reg_type))==-1)
      msg(MSG_WARN,"error replying to task %d",id);
  } else if (Reply(id, &rep, sizeof(reply_type))==-1)
    msg(MSG_WARN,"error replying to task %d",id);

  return(rep);
}
