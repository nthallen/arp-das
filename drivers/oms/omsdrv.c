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
#include <signal.h>
#include <time.h>
#include "nortlib.h"
#include "oui.h"
#include "omsdrv.h"
#include "omsint.h"
#include "tm.h"

#define CMD_PULSE_CODE (_PULSE_CODE_MINAVAIL + 0)
#define TM_PULSE_CODE (_PULSE_CODE_MINAVAIL + 1)
#define EXPINT_PULSE_CODE (_PULSE_CODE_MINAVAIL + 2)
#define TIMER_PULSE_CODE (_PULSE_CODE_MINAVAIL + 3)
#define MAX_IRQ_104 12

char omsdrv_c_id[] = "$UID: seteuid.oui,v $";
static reqqueue *free_queue;
static int oms_irq = 0;
static int oms_iid;
static int cmd_fd = -1;
static struct sigevent cmd_event;
static timer_id oms_timer;

OMS_TM_Data OMS_status;

readreq * allocate_request( char *cmd, int type, char *hdr ) {
  char *hdrsrc;
  readreq *req;
  
  if ( cmd == NULL )
    nl_error( 4, "NULL cmd in allocate_request" );
  
  req = dequeue_req( free_queue );
  if ( req == NULL ) req = malloc(sizeof(readreq));
  req->req_type = type;
  strncpy( req->cmd, cmd, IBUF_SIZE-1 );
  req->cmd[IBUF_SIZE-1] = '\0';
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
                char *cmd, void *tmdata, int size,
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
    allocate_request( cmd, OMSREQ_PROCESSTM, NULL );
  // newreq->cmd = nl_strdup( cmd );
  newreq->pending = 0;
  nlr = set_response( 1 );
  newreq->req->u.tm.handler = handler;
  newreq->req->u.tm.tmid =
    Col_send_init( name, tmdata, size, 0 );
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
    nl_error( -2, "Enqueueing TM request %d", value);
    tm_data_req *tmd = tm_data[value];
    enqueue_req( pending_queue, tmd->req );
    pq_check();
  }
}

/**
 * Allocates a readreq and enqueues it.
 */
void new_request( char *cmd, int type, char *hdr ) {
  readreq *req =
        allocate_request( cmd, type, hdr);
  enqueue_req( pending_queue, req );
  pq_check();
}

/**
 * Data handler for data received from the OMS78
 */
void handle_recv_data( void ) {
  switch ( current_req->req_type ) {
    case OMSREQ_IGNORE: break;
    case OMSREQ_PROCESSTM:
      (req->u.tm.handler)(req);
      Col_send( req->u.tm.tmid );
      break;
    case OMSREQ_LOG:
      nl_error( 0, "%s: %s", req->u.hdr, req->ibuf );
      break;
    default:
      nl_error( 4, "Invalid req_type in handle_recv_data: %d", req->req_type );
  }
  pq_recycle();
}

static int service_cmd(void) {
  char ibuf[IBUF_SIZE];
  for (;;) {
    int rv;
    rv = read(cmd_fd, ibuf, IBUF_SIZE-1);
    if ( rv > 0 ) {
      ibuf[rv] = '\0';
      nl_error(-2, "Received command '%s'", ibuf);
      if ( ibuf[0] == 'W' ) {
        ibuf[rv] = '\0';
        // oms_queue_output( ibuf+1 );
        new_request(ibuf+1, OMSREQ_NO_RESPONSE, NULL );
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
  // nl_error(-3, "Received pulse code %d value %d", code, value );
  switch (code) {
    case EXPINT_PULSE_CODE:
      service_int();
      if (InterruptUnmask(oms_irq, oms_iid) < 0)
        nl_error(1, "Error %d from InterruptUnmask", errno);
      break;
    case CMD_PULSE_CODE: return service_cmd();
    case TM_PULSE_CODE: service_tm(value); break;
    case TIMER_PULSE_CODE:
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
}

static void open_cmd_fd( int coid, short code, int value ) {
  int old_response = set_response(0);
  char *cmddev = tm_dev_name("cmd/oms");
  cmd_fd = tm_open_name(cmddev, NULL, O_RDONLY|O_NONBLOCK);
  set_response(old_response);
  if ( cmd_fd < 0 ) {
    nl_error(3, "Unable to open command channel" );
  }
  nl_error(-2, "Command fd %d opened to %s", cmd_fd, cmddev );

  /* Initialize cmd event */
  cmd_event.sigev_notify = SIGEV_PULSE;
  cmd_event.sigev_coid = coid;
  cmd_event.sigev_priority = SIGEV_PULSE_PRIO_INHERIT;
  cmd_event.sigev_code = code;
  cmd_event.sigev_value.sival_int = value;
  service_cmd();
}

static void close_cmd_fd(void) {
  close(cmd_fd);
}

static void setup_timer( int coid, int code, timer_t *timer ) {
  struct sigevent tmr_event;
  
  /* Initialize tmr event */
  tmr_event.sigev_notify = SIGEV_PULSE;
  tmr_event.sigev_coid = coid;
  tmr_event.sigev_priority = SIGEV_PULSE_PRIO_INHERIT;
  tmr_event.sigev_code = code;
  tmr_event.sigev_value.sival_int = 0;
  if ( timer_create(CLOCK_REALTIME, &tmr_event, timer) )
    nl_error( 3, "timer_create() failed: %s", strerror(errno) );
}

void set_oms_timeout( int ms ) {
  struct itimerspec ts;
  ts.it_value.tv_sec = 0;
  ts.it_value.tv_nsec = ms*1000000L;
  ts.it_interval.tv_sec = 0;
  ts.it_interval.tv_nsec = 0;
  if ( timer_settime(oms_timer, 0, &ts, NULL) != 0 )
    nl_error( 2, "Error from timer_settime(): %s", strerror(errno) );
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
  ThreadCtl(_NTO_TCTL_IO,0);
  iid = InterruptAttachEvent(oms_irq, &intr_event,
      _NTO_INTR_FLAGS_PROCESS | _NTO_INTR_FLAGS_TRK_MSK );
  if (iid == -1)
    nl_error( 3, "Unable to attach IRQ %d: errno %d", irq, errno);
  return iid;
}

int parse_spaces( char * const *p, int required ) {
  char *s = *p;
  if ( required && !isspace(*s)) return 1;
  while (isspace(*s)) ++s;
  *p = s;
  return 0;
}

int parse_long( char * const *p, long *lval ) {
  int sign = 1;
  long val = 0L;
  char *s = *p;
  if ( *s == '-' ) {
    sign = -1;
    ++s;
  }
  if ( !isdigit(*s) ) return 1;
  while (isdigit(*s)) {
    val = val * 10 + *s++ - '0';
  }
  *lval = val*sign;
  *p = s;
  return 0;
}

int parse_char( char * const *p, char c ) {
  if ( **p == c ) {
    ++(*p);
    return 0;
  }
  return 1;
}

int parse_status( char * const *p, unsigned char *status ) {
  *status = 0;
  return
    parse_status_bit( &p, status, "MP", OMS_STAT_DIR_OUT ) ||
    parse_status_bit( &p, status, "ND", OMS_STAT_DONE) ||
    parse_status_bit( &p, status, "NL", OMS_STAT_LIMIT) ||
    parse_status_bit( &p, status, "NH", OMS_STAT_HOME );
}

/**
 * Handler for TM data read from the OMS78
 */
static void parse_tm_data( readreq *req ) {
  char *p;
  int i = 0;

  // The request string is AARPRI, so I'm expecting
  // \d+,\d+,\d+,\d+\n\n\r\r
  // [PM][DN][LN][HN],[PM][DN][LN][HN],[PM][DN][LN][HN],[PM][DN][LN][HN]\n\r\r
  // [PM] direction
  // [DN] done
  // [LN] limit in this direction
  // [HN] home
  nl_error( -2, "parse_tm_data: '%s'", quote_np(req->ibuf) );
  p = req->ibuf;
  if ( parse_spaces(&p, 0) ||
       parse_long(&p, &OMS_status.step[0] ) || parse_char(&p, ',' ) ||
       parse_long(&p, &OMS_status.step[1] ) || parse_char(&p, ',' ) ||
       parse_long(&p, &OMS_status.step[2] ) || parse_char(&p, ',' ) ||
       parse_long(&p, &OMS_status.step[3] ) || parse_spaces(&p, 1) ||
       parse_status(&p,&OMS_status.status[0] ) || parse_char(&p, ',') ||
       parse_status(&p,&OMS_status.status[1] ) || parse_char(&p, ',') ||
       parse_status(&p,&OMS_status.status[2] ) || parse_char(&p, ',') ||
       parse_status(&p,&OMS_status.status[3] ) || parse_spaces(&p, 1) ) {
    nl_error( 2, "Parse error in tm data: %s", quote_np(req->ibuf) );
  }
}

#define QBUF_SIZE (IBUF_SIZE*4)
char *quote_np(char *s) {
  static qbuf[QBUF_SIZE];
  unsigned oi = 0;
  while ( oi < QBUF_SIZE-4 && *s != '\0' ) {
    if ( isprint(*s) ) {
      qbuf[oi++] = *s;
    } else {
      if ( oi < QBUF_SIZE-8 ) {
        int n;
        n = sprintf(&qbuf[oi], QBUF_SIZE-4-oi, "<%02X>", *s);
        oi += n;
      } else break;
    }
    ++s;
  }
  if ( *s != '\0' ) {
    nl_assert(oi <= QBUF_SIZE-4);
    qbuf[oi++] = '.';
    qbuf[oi++] = '.';
    qbuf[oi++] = '.';
  }
  qbuf[oi] = '\0';
  return qbuf;
}

int main( int argc, char **argv ) {
  int chid, coid;
  
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

  oms_iid = setup_interrupt( oms_irq, coid, EXPINT_PULSE_CODE );
  setup_timer( coid, TIMER_PULSE_CODE, &oms_timer );

  // Initialize the board
  out8( PC68_CONTROL, 0xA0 ); /* IRQ_E & IBF_E & !TBE_E */

  open_cmd_fd( coid, CMD_PULSE_CODE, 0 );
  init_tm_request( "OMS_status", coid, "AARPRI", 2,
      &OMS_status, sizeof(OMS_status), parse_tm_data);
  
  new_request( "", OMSREQ_IGNORE, NULL );
  new_request( "\004WY", OMSREQ_LOG, "PC68 ID" );

  // Initialize speed for X and Y; No return
  new_request( "AXVL1000;AYVL480;", OMSREQ_NO_RESPONSE, NULL );
  nl_error( 0, "Initialized" );
  
  operate(chid);

  nl_error( 0, "Shutting Down" );
  InterruptDetach(oms_iid);
  close_cmd_fd();
  shutdown_tm_requests();
  nl_error( 0, "Terminated" );
  return 0;
}

void oms_init_options( int argc, char **argv ) {
  int c;

  optind = OPTIND_RESET; /* start from the beginning */
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
