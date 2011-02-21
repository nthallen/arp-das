/* 
    Soldrv.c drives the solenoids from the compiled solenoid format file.
    Written April 9, 1987
    Modified for more complexities March 24, 1988
    Upgraded to Lattice V6.0 April 13, 1990
    Modified by Eil July 1991 for QNX.
    July 1991: soldrv does not yet accept multiple SOL_STROBES and SOL_DTOA commands.
    July 1991: soldrv does not yet send MULTCMDS to scdc.
    Ported to QNX 4 by Eil 4/20/92.
*/

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <i86.h>
#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/psinfo.h>
#include <sys/name.h>
#include <time.h>
#include <signal.h>
#include <cmdctrl.h>
#include <globmsg.h>
#include <subbus.h>
#include <das_utils.h>
#include <scdc.h>
#include "solcall.h"
#include "soldrv.h"
#include "codes.h"

#ifndef SYSTEM_BOARD
#include <sys/timers.h>
#include <sys/proxy.h>

pid_t proxy;
timer_t timer=0;
struct itimercb tcb;
struct itimerspec tval, otval;

#define DOWN_TIMER { \
  tval.it_value.tv_sec = 0L; \
  tval.it_value.tv_nsec = 0L; \
  if (reltimer(timer, &tval, 0) == -1) \
    msg(MSG_EXIT_ABNORM, "Error in reltimer"); \
}
#define UP_TIMER \
  if (reltimer(timer, &tval, 0) == -1) \
    msg(MSG_EXIT_ABNORM, "Error in reltimer")
#define RESOLUTION_TIMER { \
  /* converts back */ \
  i=count/16384; \
  j=(count%16384)/16384; \
  /* set timer resolution */ \
  tval.it_value.tv_sec = (long)i; \
  tval.it_value.tv_nsec = (long)j; \
  tval.it_interval.tv_sec = (long)i; \
  tval.it_interval.tv_nsec = (long)j; \
}
#define ATTACH_TIMER { \
  if ( (proxy = qnx_proxy_attach(0, 0, 0, 19))==-1) \
    msg(MSG_EXIT_ABNORM, "Error attaching proxy"); \
  tcb.itcb_event.evt_value = proxy; \
  if ( (timer = mktimer(TIMEOFDAY, _TNOTIFY_PROXY, &tcb))==-1) \
    msg(MSG_EXIT_ABNORM, "Error making timer"); \
}
#define DETACH_TIMER { \
  if (timer) rmtimer(timer); \
  if (proxy) qnx_proxy_detach(proxy); \
}
#else
#include <system.h>

pid_t sys_tid;

#endif

/* defines */
#define HDR "sol"
#define OPT_MINE "t:q"
#define ST_NO_MODE 0
#define ST_NEW_MODE 1
#define ST_MODE 2
#define ST_IN_MODE 0
#define ST_WAITS 1

/* function declarations */
void waitmsg(int);
void my_signalfunction(int sig_number) {
  DETACH_TIMER;
  signalfunction(sig_number);
}

/* global variables */
char *opt_string=OPT_MSG_INIT OPT_MINE OPT_BREAK_INIT OPT_CC_INIT;
int state;
int mode_request;
int crnt_mode, new_mode;
dasc_msg_type dasc_msg = { DASCMD, 0, 0 };
pid_t scdc_tid;
unsigned char dct = DCT_SOLDRV;
char replymsg;


void main(int argc, char **argv) {

/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

/* local variables */
char name[FILENAME_MAX+1];
int i, j, exp_ext, ip, count, mode_mode;
char filename[40];
char buf[MAX_MSG_SIZE];
reply_type replycode;
pid_t cmd_tid;


  /* initialise msg options from command line */
  msg_init_options(HDR,argc,argv);
  BEGIN_MSG;
  break_init_options(argc,argv);
  signal(SIGQUIT,my_signalfunction);
  signal(SIGINT,my_signalfunction);
  signal(SIGTERM,my_signalfunction);

  /* initialisations */
  mode_request = 0;
  state = ST_NO_MODE;

  j=0;
#ifdef SYSTEM_BOARD
  j=1;
#endif

  /* process args */
  opterr = 0;
  do {
    i=getopt(argc,argv,opt_string);
    switch (i) {
      case 'q': if (j)
            msg(MSG,"uses a timer on the Harvard system controller board");
          else
            msg(MSG,"uses a QNX timer");
          DONE_MSG;
          break;
      case 't': dct = atoi(optarg); break;
      case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
      default: break;
    }
  } while (i!=-1);

  if (optind >= argc)
    msg(MSG_EXIT_ABNORM,"No solenoid file specified on command line");

  if (!load_subbus())  msg(MSG_EXIT_ABNORM,"Requires resident Subbus Library");

  for (exp_ext = 0; optind < argc; optind++) {
  if (exp_ext != 0)  msg(MSG_EXIT_ABNORM,"Too many input files");
  for (j = 0; argv[optind][j] != '\0'; j++) {
    if (argv[optind][j] == '/') exp_ext = 0;
    else if (argv[optind][j] == '.') exp_ext = j;
    filename[j] = argv[optind][j];
  }
  if (exp_ext == 0) strcpy(filename+j, ".sft");
  else filename[j] = '\0';
  exp_ext = 1;
  i = read_sft(filename);
  if (i==1 || i==-1)
    msg(MSG_EXIT_ABNORM,"Can't open %s or wrong version",filename);
  }

  /* attach timer */
  ATTACH_TIMER;

  /* Look for SCDC */
  if ( (scdc_tid = qnx_name_locate(getnid(),LOCAL_SYMNAME(SCDC,name),0,0)) == -1)
    msg(MSG_EXIT_ABNORM,"Can't find %s on node %d",name,getnid());

  /* Look for cmdctrl */
  dasc_msg.dascmd.type = dct;
  cmd_tid=cc_init_options(argc,argv,dct,dct,0,0,QUIT_ON_QUIT);

  /* give priority to timer messages */
  qnx_pflags(~0,_PPF_PRIORITY_REC,0,0);

  /* program code */
  for (;;) {
    switch (state) {
      case ST_NO_MODE:
        /* wait for a msg from anyone except timer */
        msg(MSG,"No Mode wait");
        waitmsg(0);
        msg(MSG,"No Mode");
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
            /* here send multiple strobe command to scdc, update ip */
            /* strnset(buf,'\0',sizeof(buf));
            buf[0] = MULTCMD; */

            /* send discreet commands to SCDC */
            dasc_msg.dascmd.type = DCT_SCDC;
            dasc_msg.dascmd.val = mode_code[++ip];
            if ( Send( scdc_tid, &dasc_msg, &replymsg, sizeof(dasc_msg), sizeof(reply_type) ) == -1)
              msg(MSG_EXIT_ABNORM,"Error sending to %s",SCDC);
            if (replymsg != DAS_OK)
              msg(MSG_EXIT_ABNORM,"Bad response from %s",SCDC);
            ip++;
            break;
          case SOL_WAIT:
            msg(MSG,"Step %d wait", ip);
            waitmsg(proxy);
            msg(MSG,"Step %d", ip);
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
            state = ST_NEW_MODE;
            DOWN_TIMER;
            new_mode = mode_code[ip+1];
            break;
          case SOL_MSWOK:
            if (mode_request != 0) {
              DOWN_TIMER;
              state = ST_NEW_MODE;
              } else ip++;
            break;
          case SOL_DTOA:
            /* here write up till 0xFF, update ip */
            write_subbus(0,set_points[mode_code[ip+1]].address,
            set_points[mode_code[ip+1]].value);
            ip += 2;
            break;
          } /* switch */
          break;
        case ST_WAITS:
          msg(MSG,"Step %d waits", ip);
          waitmsg(proxy);
          msg(MSG,"Step %d", ip);
          if (--count <= 0) mode_mode = ST_IN_MODE;
      
        } /* switch */
      } /* switch */
    } /* for(;;) */

} /* main */


/*
    Wait for msg from (cmdctrl or anyone) or from the timer.
    Always accept a cmdctrl msg or any msg that has correct structure.
    Waiting for cmdctrl msg and recieving timer msg is an error.
*/
void waitmsg ( int t ) {
int r;

assert(t==0 || t==proxy);

  do {
    replymsg=DAS_OK;
    if ( (r=Receive(0, &dasc_msg, sizeof(dasc_msg))) == -1)
      msg(MSG_WARN,"error recieving messages");
    else if (r != proxy) {  /* anyone except timer */
      if (dasc_msg.dascmd.type != dct ||
        dasc_msg.dascmd.val >= n_modes ||
          mode_indices[dasc_msg.dascmd.val] == -1) {
        /* bad received msg */
        replymsg = DAS_UNKN;
        if (Reply(r,&replymsg,sizeof(reply_type))==-1)
          msg(MSG_WARN,"can't reply UNKNOWN to task %d",r);
        else msg(MSG_WARN,"replied UNKNOWN to task %d",r);
      } else {
        /* good received msg */
        if (Reply(r,&replymsg,sizeof(reply_type))==-1)
          msg(MSG_WARN,"can't reply to task %d",r);
        if (state == ST_MODE && dasc_msg.dascmd.val == 0) {
          DOWN_TIMER;
          state = ST_NEW_MODE;
          new_mode = 0;
        } else {
          new_mode = dasc_msg.dascmd.val;
          if (state == ST_NO_MODE) state = ST_NEW_MODE;
          else {
            mode_request = 1;
            msg(MSG,"Mode %d: %d Pending",crnt_mode, new_mode);
          }
        }
      }
    }
    /* msg from timer, don't reply to a proxy */
    else if (!t) msg(MSG_EXIT_ABNORM,"timer screwed up");
  }  while (r==-1 || replymsg!=DAS_OK);
}
