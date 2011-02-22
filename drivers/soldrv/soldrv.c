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
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <i86.h>
#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/sendmx.h>
#include <sys/psinfo.h>
#include <sys/name.h>
#include <time.h>
#include <signal.h>
#include "cmdctrl.h"
#include "globmsg.h"
#include "subbus.h"
#include "das.h"
#include "eillib.h"
#include "nortlib.h"
#include "scdc.h"
#include "soldrv.h"
#include "sol.h"
#include "codes.h"
#include "da_cache.h"

#ifndef SYSTEM_BOARD
#include <sys/timers.h>
#include <sys/proxy.h>

pid_t timer_proxy;
timer_t timer=0;
struct itimercb tcb;
struct itimerspec tval, otval;
unsigned int stat = 0;

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
#define ATTACH_TIMER { \
    if ( (timer_proxy = qnx_proxy_attach(0, 0, 0, -1))==-1) \
	msg(MSG_EXIT_ABNORM, "Error attaching timer proxy"); \
    tcb.itcb_event.evt_value = timer_proxy; \
    if ( (timer = mktimer(TIMEOFDAY, _TNOTIFY_PROXY, &tcb))==-1) \
	msg(MSG_EXIT_ABNORM, "Error making timer"); \
}
#define DETACH_TIMER { \
    if (timer) rmtimer(timer); \
    if (timer_proxy) qnx_proxy_detach(timer_proxy); \
}
#else
#include <system.h>

pid_t sys_tid;

#endif

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
char *opt_string=OPT_MSG_INIT OPT_MINE OPT_CC_INIT;
int state;
int mode_request;
int crnt_mode, new_mode;
pid_t scdc_tid;
unsigned char dct = DCT_SOLDRV_A;
pid_t *proxy_pids;
unsigned char reg_which;

void main(int argc, char **argv) {

  /* getopt variables */
  extern char *optarg;
  extern int optind, opterr, optopt;

  /* local variables */
  char name[FILENAME_MAX+1];
  int i, exp_ext, ip, mode_mode;
  long j;
  unsigned int count;
  char filename[40];
  reply_type replycode;
  pid_t cmd_tid;
  struct _mxfer_entry smsg[3];
  struct _mxfer_entry rmsg;
  msg_hdr_type hdr_mult = SC_MULTCMD;
  msg_hdr_type hdr_dasc = DASCMD;
  unsigned char type_scdc = DCT_SCDC;
  unsigned short cacher = CACHE_ADDR;

  /* initialise msg options from command line */
  msg_init_options(HDR,argc,argv);
  BEGIN_MSG;
  signal(SIGQUIT,my_signalfunction);
  signal(SIGINT,my_signalfunction);
  signal(SIGTERM,my_signalfunction);

  /* initialisations */
  mode_request = 0;
  state = ST_NO_MODE;
  _setmx(&rmsg,&replycode,sizeof(reply_type));

  j=0;
#ifdef SYSTEM_BOARD
  j=1;
#endif

  /* process args */
  opterr = 0;
  optind = 0;
  do {
    i=getopt(argc,argv,opt_string);
    switch (i) {
    case 'd': cacher = atoh(optarg); break;
    case 'q': if (j)
      msg(MSG,"uses timer board timer");
    else
      msg(MSG,"uses a QNX timer");
      DONE_MSG;
      break;
    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
    default: break;
    }
  } while (i!=-1);

  if (optind >= argc)
    msg(MSG_EXIT_ABNORM,"No solenoid file specified on command line");

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
    if (i==1) msg(MSG_EXIT_ABNORM,"Can't open %s",basename(filename));
    if (i==-1) msg(MSG_EXIT_ABNORM,"Wrong version: %s",basename(filename));
  }

  switch (toupper(which)) {
  case 'A': reg_which = SOLDRV_PROXY_A; dct = DCT_SOLDRV_A;  break;
  case 'B': reg_which = SOLDRV_PROXY_B; dct = DCT_SOLDRV_B;  break;
  case 'C': reg_which = SOLDRV_PROXY_C; dct = DCT_SOLDRV_C;  break;
  case 'D': reg_which = SOLDRV_PROXY_D; dct = DCT_SOLDRV_D;  break;
  case 'E': reg_which = SOLDRV_PROXY_E; dct = DCT_SOLDRV_E;  break;
  case 'F': reg_which = SOLDRV_PROXY_F; dct = DCT_SOLDRV_F;  break;
  case 'G': reg_which = SOLDRV_PROXY_G; dct = DCT_SOLDRV_G;  break;
  case 'H': reg_which = SOLDRV_PROXY_H; dct = DCT_SOLDRV_H;  break;
  case 'I': reg_which = SOLDRV_PROXY_I; dct = DCT_SOLDRV_I;  break;
  case 'J': reg_which = SOLDRV_PROXY_J; dct = DCT_SOLDRV_J;  break;
  default: msg(MSG_EXIT_ABNORM,"unknown SOLDRV_PROXY type %s", optarg);
  }

  /* attach timer */
  ATTACH_TIMER;

  /* look for subbus */
  if (n_set_points) {
    set_response(NLRSP_QUIET);	/* for norts Col_set_pointer */    		
    for (j=0,i=0;i<n_set_points;i++) {
      if (set_points[i].address==0) {
	if (j++ == 0) { /* Only need to register once */
	  if (!(Col_set_pointer(dct, &stat, 0)))
	    msg(MSG,"achieved cooperation with DG");
	  else msg(MSG_WARN,"Can't cooperate with DG");
	}
      } else if (set_points[i].address >= cacher) {
        j++;
      }
    }
    if (n_set_points > j) {	
      if (seteuid(0)==-1) msg(MSG_EXIT_ABNORM,"Can't set euid to root");
      if (!load_subbus())
	msg(MSG_EXIT_ABNORM,"Requires resident Subbus Library");
    }
  }

  /* Look for SCDC */
  if (n_solenoids)
    if ( (scdc_tid = qnx_name_locate(getnid(),LOCAL_SYMNAME(SCDC),0,0)) == -1)
      msg(MSG_EXIT_ABNORM,"Can't find symbolic name for %s",SCDC);

  /* Look for cmdctrl */
  cmd_tid=cc_init_options(argc,argv,dct,dct,reg_which,reg_which,QUIT_ON_QUIT);

  /* sort messages by priority */
  /*    qnx_pflags(~0,_PPF_PRIORITY_REC,0,0);*/

  /* set up proxies */
  if (n_proxies) 
    if ( !(proxy_pids = (pid_t *) calloc(n_proxies, sizeof(pid_t))))
      msg(MSG_EXIT_ABNORM,"Can't allocate space for proxies");


  /* program code */
  for (;;) {
    switch (state) {
    case ST_NO_MODE:
      /* wait for a msg from anyone except timer */
      msg(MSG,"No Mode");
      waitmsg(0);
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
	  /* send discreet commands to SCDC */
	  _setmx(&smsg[0],&hdr_dasc,sizeof(msg_hdr_type));
	  _setmx(&smsg[1],&type_scdc,sizeof(unsigned char));
	  ip++;
	  _setmx(&smsg[2],&mode_code[ip],sizeof(unsigned char));
	  while ( Sendmx( scdc_tid, 3, 1, smsg, &rmsg) ==-1)
	    msg((errno==EINTR) ? MSG_WARN : MSG_EXIT_ABNORM,"Error sending to %s",SCDC);
	  if (replycode != DAS_OK)
	    msg(MSG_WARN,"Bad response from %s",SCDC);
	  ip++;
	  break;
	case SOL_MULT_STROBES:
	  _setmx(&smsg[0],&hdr_mult,sizeof(msg_hdr_type));
	  i = mode_code[++ip];
	  if (mode_code[ip] > (MAX_MSG_SIZE-2)) {
	    msg(MSG_WARN,"multiple strobe command of length %d too long, shortened",mode_code[ip]+2);
	    i = MAX_MSG_SIZE - 2;
	  }
	  _setmx(&smsg[1],&i,1);
	  ip++;
	  _setmx(&smsg[2],&mode_code[ip],i);
	  while (Sendmx(scdc_tid,3,1,smsg,&rmsg)==-1)
	    msg((errno==EINTR) ? MSG_WARN : MSG_EXIT_ABNORM,"Error sending to %s",SCDC);
	  if (replycode != DAS_OK)
	    msg(MSG_WARN,"Bad response from %s",SCDC);
	  ip+=mode_code[ip-1];
	  break;
	case SOL_WAIT:
	  msg(MSG_DEBUG,"Step %d wait", ip);
	  waitmsg(timer_proxy);
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
	    stat=set_points[mode_code[ip+1]].value;
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
	waitmsg(timer_proxy);
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

void waitmsg ( pid_t invite ) {
struct {
    msg_hdr_type msg_hdr;
    union {
	dascmd_type dasc_msg;
	proxy_reg_type proxy_reg;
    } u;
} sol_msg;
int i;
pid_t recv_pid;
msg_hdr_type replymsg;

assert(invite==0 || invite==timer_proxy);

    do {
	sol_msg.msg_hdr = DEATH;
	replymsg=DAS_OK;
	if ( (recv_pid=Receive(0, &sol_msg, sizeof(sol_msg))) == -1)
	    msg(MSG_WARN,"error recieving messages");
	else if (recv_pid != timer_proxy) {  /* anyone except timer */
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
	} /* if */
	/* msg from timer, don't reply to a timer proxy */
	else if (!invite) msg(MSG_EXIT_ABNORM,"timer screwed up");
    }  while (recv_pid==-1 || replymsg!=DAS_OK || (invite==timer_proxy && recv_pid!=timer_proxy));
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
} else 
    if (Reply(id, &rep, sizeof(reply_type))==-1)
	msg(MSG_WARN,"error replying to task %d",id);

return(rep);
}
