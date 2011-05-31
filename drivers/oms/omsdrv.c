#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <sys/irqinfo.h>
#include <conio.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "nortlib.h"
#include "oui.h"
#include "omsdrv.h"
#include "omsint.h"
#include "globmsg.h"

#define TM_PROXY_MSG 255
static reqqueue *free_queue;
static int oms_irq = 0;
char omsdrv_c_id[] = "$UID: seteuid.oui,v $";

readreq * allocate_request( char *cmd, int type,
	int n_req, pid_t sender, char *hdr ) {
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
	case OMSREQ_SENDTM: break;
	case OMSREQ_LOG:
	  hdrsrc = hdr == NULL ? cmd : hdr;
	  strncpy( req->u.hdr, hdrsrc, IBUF_SIZE-1 );
	  req->u.hdr[IBUF_SIZE-1] = '\0';
	  break;
	case OMSREQ_REPLY: req->u.sender = sender; break;
	default:
	  nl_error( 4, "Invalid type in allocate_request" );
  }
  return req;
}

/*
  The requests are supposed to be static, presumably initialized
  once at the beginning. There will probably be one TM req with
  multiple sub-requests. On init the req must be created, then
  registered with collection and a proxy registered with
  collection. When the proxy is received, the request should
  be enqueued and the command issued. When complete, the result
  needs to be sent to TM. The request structure here holds the
  tmid, but it doesn't include any larger context, specifically
  where we are in the acquisition process or what the command is.
  A TM request needs to know
    The string DG associates with this data
	The number DG associates with the proxy (magic)
	
	The tmid returned from collection
	The proxy returned from collection
	The command string that needs to be issued
	Whether we are processing the request or not.
*/
#define MAX_TM_REQUESTS 4
static tm_data_req *tm_data[MAX_TM_REQUESTS];
static int n_tm_requests;
static void init_tm_request( char *name, int size, int coid,
				char *cmd, int n_req ) {
  int nlr;
  tm_data_req *newreq;
  
  if ( n_tm_requests >= MAX_TM_REQUESTS ) {
	nl_error( 2, "Too many calls to init_tm_request" );
	return;
  }
  newreq = tm_data[n_tm_requests++] =
      new_memory(sizeof(tm_data_req));
  newreq->req = allocate_request( cmd, OMSREQ_SENDTM, n_req, 0,
		      NULL );
  newreq->cmd = nl_strdup( cmd );
  newreq->pending = 0;
  nlr = set_response( 1 );
  newreq->req->u.tmid = Col_send_init( name, &newreq->req->ibuf, size );
  if ( newreq->req->u.tmid != NULL )
    Col_send_arm(newreq->req->u.tmid, coid, TM_PULSE_CODE, n_tm_requests-1);
  set_response( nlr );
}

static void oms_queue_output( char *cmd ) {
  while ( *cmd != '\0' ) enqueue_char(output_queue, *cmd++);
}

static void service_tm(int value) {
  if ( value < n_tm_requests ) {
    tm_data_req *tm_data = tm_data[value];
    enqueue_req( pending_queue, tm_data->req );
    oms_queue_output( tm_data->cmd );
  }
}

void new_request( char *cmd, int type, int n_req, pid_t sender,
	char *hdr ) {
  readreq *req =
	allocate_request( cmd, type, n_req, sender, hdr);
  enqueue_req( pending_queue, req );
  oms_queue_output( cmd );
}

void handle_recv_proxy( void ) {
  readreq *req;
  Reply_hdr_from_oms rep_hdr;
  struct _mxfer_entry mx[2];
  
  while ( (req = dequeue_req( satisfied_queue )) != NULL ) {
    switch ( req->req_type ) {
      case OMSREQ_IGNORE: break;
	// ### parse the data into the tm structure
	Col_send( req->u.tmid );
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

static oms_done = 0;

/* operate() houses the main Receive Loop for oms driver */
void operate( void ) {
  while ( ! oms_done ) {
   if ( MsgReceivePulse(chid, &pulse, sizeof(pulse), NULL) == 0 ) {
     done = service_pulse(pulse.code, pulse.value.sival_int);
   } else switch (errno) {
     case EFAULT:
     case EINTR:
     case ESRCH:
     case ETIMEDOUT:
     default:
       nl_error( 2, "Error %d from MsgReceivePulse", errno);
       break;
   }
    who = Receive( 0, &cmd, sizeof( cmd ) );
    if ( who != -1 ) {
      if ( who == recv_proxy ) handle_recv_proxy();
      else {
	int i;
	for ( i = 0; i < n_tm_requests; i++ ) {
	  if ( who == tm_data[i]->tm_proxy ) {
	    handle_tm_proxy( tm_data[i] );
	    break;
	  }
	}
	if ( i >= n_tm_requests ) {
	  if ( cmd.hdr.signature != OMS_SIG )
	    reply_byte( who, REP_UNKN );
	  else {
	    rep.hdr.status = 0;
	    switch ( cmd.hdr.function ) {
	      case OMSMSG_READ:
		new_request( cmd.command, OMSREQ_REPLY, cmd.hdr.n_req,
					who, NULL);
		continue; /* don't reply yet... */
	      case OMSMSG_READ_LOG:
		new_request( cmd.command, OMSREQ_LOG, cmd.hdr.n_req,
					0, NULL);
		break;
	      case OMSMSG_READ_IGNORE:
		new_request( cmd.command, OMSREQ_IGNORE, cmd.hdr.n_req,
					who, NULL);
		break;
	      case OMSMSG_WRITE:
		nl_error( -2, "Write: \"%s\"", cmd.command );
		oms_queue_output( cmd.command );
		break;
	      case OMSMSG_QUIT:
		oms_done = 1;
		break;
	      default:
		rep.hdr.status = 1;
		break;
	    }
	    Reply( who, &rep, sizeof(rep) );
	  }
	}
      }
    }
  }
}

int main( int argc, char **argv ) {
  int omsname_id;
  char *name;
  int chid, coid;
  
  oui_init_options( argc, argv );

  pending_queue = new_reqqueue(10);
  satisfied_queue = new_reqqueue(10);
  free_queue = new_reqqueue(10);
  output_queue = new_charqueue(80);

  // Set up interrupt event
  // Initialize command channel
  // Initialize telemetry channel

  outp( PC68_CONTROL, 0xA0 ); /* IRQ_E & IBF_E & !TBE_E */
  
  /* attach oms driver name */

  /* Initialize channel to receive pulses*/
  chid = ChannelCreate(0);
  if ( chid == -1 )
    nl_error( 3, "Error %d from ChannelCreate()", errno );
  coid = ConnectAttach( ND_LOCAL_NODE, 0, chid, _NTO_SIDE_CHANNEL, 0);
  init_tm_request( "OMS_stetus", OMS_CMD_MAX, coid, "AARPQA", 8 );
  new_request( "\004WY", OMSREQ_LOG, 1, 0, "PC68 ID" );

  // Initialize speed for X and Y; No return
  oms_queue_output( "AXVL1000;AYVL480;" );
  nl_error( 0, "Initialized" );
  
  operate();
  nl_error( 0, "Shutting Down" );

  // ### detach interrupt
  // ### close command channel
  // ### close telemetry channel

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
	  case 'q':
		oms_shutdown();
		exit(0);
	  case '?':
		nl_error(3, "Unrecognized Option -%c", optopt);
	  default:
		break;
	}
  }
  if ( oms_irq == 0 )
	nl_error( 3, "Must specify an IRQ" );
}
