#include <sys/types.h>
#include <sys/neutrino.h>
#include <sys/iomsg.h>
#include <sys/netmgr.h>
#include <hw/inout.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "nortlib.h"
#include "oui.h"
#include "omsdrv.h"
#include "omsint.h"
#include "tm.h"

#define CMD_PULSE_CODE (_PULSE_CODE_MINAVAIL + 0)
#define TM_PULSE_CODE (_PULSE_CODE_MINAVAIL + 1)
#define EXPINT_PULSE_CODE (_PULSE_CODE_MINAVAIL + 2)
#define MAX_IRQ_104 12

char omsdrv_c_id[] = "$UID: seteuid.oui,v $";
static reqqueue *free_queue;
static int oms_irq = 0;
static int cmd_fd = -1;
static struct sigevent cmd_event;

OMS_TM_Data OMS_status;

readreq * allocate_request( char *cmd, int type,
		      int n_req, char *hdr ) {
  char *hdrsrc;
  readreq *req;
  
  if ( cmd == NULL )
	nl_error( 4, "NULL cmd in allocate_request" );
  
  req = dequeue_req( free_queue );
  if ( req == NULL ) req = malloc(sizeof(readreq));
  req->req_type = type;
  req->n_req = n_req;
  switch ( type ) {
    case OMSREQ_IGNORE: break;
    case OMSREQ_PROCESSTM: break;
    case OMSREQ_LOG:
      hdrsrc = hdr == NULL ? cmd : hdr;
      strncpy( req->u.hdr, hdrsrc, IBUF_SIZE-1 );
      req->u.hdr[IBUF_SIZE-1] = '\0';
      break;
    default:
      nl_error( 4, "Invalid type in allocate_request" );
  }
  return req;
}

/*
  The requests are supposed to be static, presumably initialized
  once at the beginning. There will probably be one TM req with
  multiple sub-requests. On init the req must be created, then
  registered with collection and armed.
  When the corresponding pulse is received, the request should
  be enqueued and the command issued. When complete, the result
  needs to be sent to TM. The request structure here holds the
  tmid, but it doesn't include any larger context, specifically
  where we are in the acquisition process or what the command is.
  A TM request needs to know
    The string DG associates with this data
    The tmid returned from collection
    The command string that needs to be issued
    Whether we are processing the request or not.
*/
#define MAX_TM_REQUESTS 4
static tm_data_req *tm_data[MAX_TM_REQUESTS];
static int n_tm_requests;

static void init_tm_request( char *name, int coid,
		char *cmd, int n_req, void *tmdata, int size,
		void (*handler)(readreq *)) {
  int nlr;
  tm_data_req *newreq;
  
  if ( n_tm_requests >= MAX_TM_REQUESTS ) {
    nl_error( 2, "Too many calls to init_tm_request" );
    return;
  }
  newreq = tm_data[n_tm_requests++] =
      new_memory(sizeof(tm_data_req));
  newreq->req =
    allocate_request( cmd, OMSREQ_PROCESSTM, n_req, NULL );
  newreq->cmd = nl_strdup( cmd );
  newreq->pending = 0;
  nlr = set_response( 1 );
  newreq->req->u.tm.handler = handler;
  newreq->req->u.tm.tmid =
    Col_send_init( name, tmdata, size, 1 );
  if ( newreq->req->u.tm.tmid != NULL )
    Col_send_arm(newreq->req->u.tm.tmid, coid, TM_PULSE_CODE,
      n_tm_requests-1);
  set_response( nlr );
}

static void shutdown_tm_requests(void) {
  int i;
  for ( i = 0; i < n_tm_requests; ++i ) {
    Col_send_reset(tm_data[i]->req->u.tm.tmid);
  }
}

static void oms_queue_output( char *cmd ) {
  while ( *cmd != '\0' ) enqueue_char(output_queue, *cmd++);
}

static void service_tm(int value) {
  if ( value < n_tm_requests ) {
    tm_data_req *tmd = tm_data[value];
    enqueue_req( pending_queue, tmd->req );
    oms_queue_output( tmd->cmd );
  }
}

void new_request( char *cmd, int type, int n_req, char *hdr ) {
  readreq *req =
	allocate_request( cmd, type, n_req, hdr);
  enqueue_req( pending_queue, req );
  oms_queue_output( cmd );
}

void handle_recv_data( void ) {
  readreq *req;
  
  while ( (req = dequeue_req( satisfied_queue )) != NULL ) {
    switch ( req->req_type ) {
      case OMSREQ_IGNORE: break;
      case OMSREQ_PROCESSTM:
	(req->u.tm.handler)(req);
	Col_send( req->u.tm.tmid );
	continue; /* TM requests are static */
      case OMSREQ_LOG:
	nl_error( 0, "%s: %s", req->u.hdr, req->ibuf );
	break;
      default:
	nl_error( 4, "Invalid req_type: %d", req->req_type );
    }
    enqueue_req( free_queue, req );
  }
}

static int service_cmd(void) {
  char ibuf[IBUF_SIZE];
  for (;;) {
    int rv;
    rv = read(cmd_fd, ibuf, IBUF_SIZE-1);
    if ( rv > 0 ) {
      ibuf[rv] = '\0';
      if ( ibuf[0] == 'W' ) {
        ibuf[rv] = '\0';
        oms_queue_output( ibuf+1 );
      } else if ( ibuf[0] == 'Q' ) {
        return 1;
      } else {
        nl_error(2, "Unknown command: '%c'", ibuf[0] );
      }
    } else if ( rv == 0 ) {
      return 1; // quit command
    } else if ( rv == -1 ) {
      if ( errno != EAGAIN && errno != EINTR)
        nl_error(3, "Received error %d from read() in check_command()", errno );
    } else nl_error(4, "Bad return value [%d] from read in check_command", rv );
    rv = ionotify(cmd_fd, _NOTIFY_ACTION_POLLARM,
        _NOTIFY_COND_INPUT, &cmd_event);
    if (rv == -1)
      nl_error( 3, "Received error %d from iontify in service_cmd()", errno );
    if (rv == 0) break;
  }
  return 0;
}

static int service_pulse( int code, int value ) {
  switch (code) {
    case EXPINT_PULSE_CODE: service_int(); break;
    case CMD_PULSE_CODE: return service_cmd();
    case TM_PULSE_CODE: service_tm(value); break;
    default:
      nl_error( 2, "Invalid pulse code: %d", code );
  }
  return 0;
}

static int oms_done = 0;

/* operate() houses the main Receive Loop for oms driver */
void operate( int chid ) {
  struct _pulse pulse;
  while ( ! oms_done ) {
    if ( MsgReceivePulse(chid, &pulse, sizeof(pulse), NULL) == 0 ) {
      oms_done = service_pulse(pulse.code, pulse.value.sival_int);
    } else switch (errno) {
      case EFAULT:
      case EINTR:
      case ESRCH:
      case ETIMEDOUT:
      default:
        nl_error( 2, "Error %d from MsgReceivePulse", errno);
        break;
    }
  }

// ### This is the old code, to be deleted.
// ### Need to write service_cmd();
//    who = Receive( 0, &cmd, sizeof( cmd ) );
//    if ( who != -1 ) {
//      if ( who == recv_proxy ) handle_recv_proxy();
//      else {
//	int i;
//	for ( i = 0; i < n_tm_requests; i++ ) {
//	  if ( who == tm_data[i]->tm_proxy ) {
//	    handle_tm_proxy( tm_data[i] );
//	    break;
//	  }
//	}
//	if ( i >= n_tm_requests ) {
//	  if ( cmd.hdr.signature != OMS_SIG )
//	    reply_byte( who, REP_UNKN );
//	  else {
//	    rep.hdr.status = 0;
//	    switch ( cmd.hdr.function ) {
//	      case OMSMSG_READ:
//		new_request( cmd.command, OMSREQ_REPLY, cmd.hdr.n_req,
//					who, NULL);
//		continue; /* don't reply yet... */
//	      case OMSMSG_READ_LOG:
//		new_request( cmd.command, OMSREQ_LOG, cmd.hdr.n_req,
//					0, NULL);
//		break;
//	      case OMSMSG_READ_IGNORE:
//		new_request( cmd.command, OMSREQ_IGNORE, cmd.hdr.n_req,
//					who, NULL);
//		break;
//	      case OMSMSG_WRITE:
//		nl_error( -2, "Write: \"%s\"", cmd.command );
//		oms_queue_output( cmd.command );
//		break;
//	      case OMSMSG_QUIT:
//		oms_done = 1;
//		break;
//	      default:
//		rep.hdr.status = 1;
//		break;
//	    }
//	    Reply( who, &rep, sizeof(rep) );
//	  }
//	}
//      }
//    }

}

static void open_cmd_fd( int coid, short code, int value ) {
  int old_response = set_response(0);
  char *cmddev = tm_dev_name("cmd/oms");
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

static int setup_interrupt( int irq, int coid, short code ) {
  struct sigevent intr_event;
  int iid;

  if ( irq < 0 || irq >= MAX_IRQ_104 )
    nl_error( 3, "IRQ %d is invalid", irq );
  intr_event.sigev_notify = SIGEV_PULSE;
  intr_event.sigev_coid = coid;
  intr_event.sigev_priority = SIGEV_PULSE_PRIO_INHERIT;
  intr_event.sigev_code = EXPINT_PULSE_CODE;
  intr_event.sigev_value.sival_int = 0;
  iid = InterruptAttachEvent(oms_irq, &intr_event,
      _NTO_INTR_FLAGS_PROCESS | _NTO_INTR_FLAGS_TRK_MSK );
  if (iid == -1)
    nl_error( 3, "Unable to attach IRQ %d: errno %d", irq, errno);
  return iid;
}

/** Parse data into the OMS_status structure
 */
static void parse_tm_data( readreq *req ) {
  char *p;
  int i = 0;

  // ######
  // The request string is AARPRI, so I'm expecting
  // \d+,\d+,\d+,\d+;
  // [PM][DN][LN][HN],[PM][DN][LN][HN],[PM][DN][LN][HN],[PM][DN][LN][HN]
  // [PM] direction
  // [DN] done (ignore)
  // [LN] limit in this direction
  // [HN] home
  p = req->ibuf;; 
  if ( ! isdigit(*p) ) {
    nl_error( 2, "Unrecognized response from OMS:" );
    return;
  }
  while (isdigit(*p) && i < 4) {
    long int pos = 0;
    while (isdigit(*p))
      pos = pos*10 + *p++ - '0';
    OMS_status.step[i++] = pos;
    switch (*p) {
      case '\0':
	nl_error(2, "Short response 1" );
	return;
      case '-':
	nl_error(2, "Negative number reported");
	return;
      case ',':
	++p;
	break;
      case ';':
	break;
    }
  }
  // Should only reach here when pointing to ';'
  // unless i == 4
  if ( *p++ != ';' ) {
    nl_error(2, "Too many values reported");
    return;
  }
  i = 0;
  while (isalpha(*p)) {
    unsigned char status = 0;
    if ( *p == 'P' ) status |= OMS_STAT_DIR;
    else if (*p != 'M') {
      nl_error( 1, "Invalid char in dir" );
    }
    if ( *++p == 'D') status |= OMS_STAT_DONE;
    else if (*p != 'N') {
      nl_error( 1, "Invalid char in done" );
    }
    if ( *++p == 'L') status |= OMS_STAT_LIMIT;
    else if (*p != 'N') {
      nl_error( 1, "Invalid char in limit" );
    }
    if ( *++p == 'H') status |= OMS_STAT_HOME;
    else if (*p != 'N') {
      nl_error( 1, "Invalid char in home" );
    }
    OMS_status.status[i++] = status;
    if ( *++p == ',' ) p++;
    else break;
  }

}

int main( int argc, char **argv ) {
  int chid, coid, iid;
  
  oui_init_options( argc, argv );

  pending_queue = new_reqqueue(10);
  satisfied_queue = new_reqqueue(10);
  free_queue = new_reqqueue(10);
  output_queue = new_charqueue(80);

  /* Initialize channel to receive pulses*/
  chid = ChannelCreate(0);
  if ( chid == -1 )
    nl_error( 3, "Error %d from ChannelCreate()", errno );
  coid = ConnectAttach( ND_LOCAL_NODE, 0, chid, _NTO_SIDE_CHANNEL, 0);

  iid = setup_interrupt( oms_irq, coid, EXPINT_PULSE_CODE );
  open_cmd_fd( coid, CMD_PULSE_CODE, 0 );
  init_tm_request( "OMS_status", coid, "AARPRI", 2,
      &OMS_status, sizeof(OMS_status), parse_tm_data);

  // Initialize the board
  out8( PC68_CONTROL, 0xA0 ); /* IRQ_E & IBF_E & !TBE_E */
  
  new_request( "\004WY", OMSREQ_LOG, 1, "PC68 ID" );

  // Initialize speed for X and Y; No return
  oms_queue_output( "AXVL1000;AYVL480;" );
  nl_error( 0, "Initialized" );
  
  operate(chid);

  nl_error( 0, "Shutting Down" );
  InterruptDetach(iid);
  close_cmd_fd();
  shutdown_tm_requests();
  nl_error( 0, "Terminated" );
  return 0;
}

void oms_init_options( int argc, char **argv ) {
  int c;

  optind = 0; /* start from the beginning */
  opterr = 0; /* disable default error message */
  while ((c = getopt(argc, argv, opt_string)) != -1) {
    switch (c) {
      case 'i':
	oms_irq = atoi(optarg);
	break;
      case '?':
	nl_error(3, "Unrecognized Option -%c", optopt);
      default:
	break;
    }
  }
  if ( oms_irq == 0 )
    nl_error( 3, "Must specify an IRQ" );
}
