#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <fcntl.h>
#include <ctype.h>
#include "serusb.h"
#include "nl_assert.h"

static char sb_ibuf[SB_SERUSB_MAX_REQUEST];
static int sb_ibuf_idx = 0;
static sbd_request_t sbdrq[SUBBUS_MAX_REQUESTS];
static sbd_request_t *cur_req;
static unsigned int sbdrq_head = 0, sbdrq_tail = 0;
static int sb_fd;
static struct sigevent ionotify_event;

static int n_writes = 0;
static int n_reads = 0;
static int n_part_reads = 0;

// Transmits a request if the currently queued
// request has not been transmitted.
static void process_request(void) {
  int cmdlen, n;
  sbd_request_t *sbr;
  if ( sbdrq_head == sbdrq_tail || cur_req != NULL)
    return;
  nl_assert( sbdrq_head < SUBBUS_MAX_REQUESTS );
  sbr = &sbdrq[sbdrq_head];
  nl_assert( sbr->status == SBDR_STATUS_QUEUED );
  switch (sbr->type) {
    case SBDR_TYPE_INTERNAL:
      switch (sbr->request[0]) {
        case '\n': // NOP
        case 'V':  // Board Revision
          break;
        default:
          nl_error( 4, "Invalid internal request" );
      }
      break;
    case SBDR_TYPE_CLIENT:
      switch (sbr->request[0]) {
        case 'R':
        case 'W':
        case 'V':
        case 'S':
        case 'C':
          break;
        default:
          nl_error( 4, "Invalid client request: '%c'", sbr->request[0] );
      }
      break;
    default:
      nl_error(4, "Invalid request type" );
  }
  cmdlen = strlen( sbr->request );
  nl_error(-2, "Request: '%*.*s'", cmdlen-1, cmdlen-1, sbr->request );
  n = write(sb_fd, sbr->request, cmdlen);
  ++n_writes;
  nl_assert( n == cmdlen );
  sbr->status = SBDR_STATUS_SENT;
  cur_req = sbr;
}

/**
 * This is where we serialize the request.
 */
static void enqueue_sbreq( int type, int rcvid, char *req ) {
  sbd_request_t *sbr = &sbdrq[sbdrq_tail];
  int i;
  int new_tail = sbdrq_tail+1;
  int old_tail;
  if ( new_tail >= SUBBUS_MAX_REQUESTS ) new_tail = 0;
  if ( new_tail == sbdrq_head )
    nl_error( 4, "Request queue overflow" );
  for ( i = 0; i < SB_SERUSB_MAX_REQUEST; ) {
    if ( req[i] == '\n' ) {
      ++i;
      break;
    } else if ( req[i] == '\0' ) {
      break;
    } else ++i;
  }
  if ( i >= SB_SERUSB_MAX_REQUEST )
    nl_error( 4, "Request exceeds %d characters", SB_SERUSB_MAX_REQUEST-1 );
  sbr->type = type;
  sbr->rcvid = rcvid;
  strncpy(sbr->request, req, i);
  sbr->request[i] = '\0';
  nl_error( -2, "Enqueued: '%*.*s'", i-1, i-1, sbr->request );
  sbr->status = SBDR_STATUS_QUEUED;
  old_tail = sbdrq_tail;
  sbdrq_tail = new_tail;
  if ( sbdrq_head == old_tail ) process_request();
}

static int sb_data_arm(void) {
  int cond;
  cond = ionotify( sb_fd, _NOTIFY_ACTION_POLLARM,
    _NOTIFY_COND_INPUT, &ionotify_event );
  if ( cond == -1 )
    nl_error( 3, "Error from ionotify: %s", strerror(errno));
  return cond;
}

// dequeue_request() sends the response to client
// requests and dequeues the current request.
static void dequeue_request( char *response, int nb ) {
  int rv;

  nl_assert( cur_req != NULL);
  switch( cur_req->type ) {
    case SBDR_TYPE_INTERNAL:
      switch (response[0]) {
        case '0': break;
        case 'V':
          nl_error( 0, "Version %s", response+1 );
          break;
        default:
          nl_error( 4, "Invalid response in dequeue_request" );
      }
      break;
    case SBDR_TYPE_CLIENT:
      rv = MsgReply( cur_req->rcvid, nb+1, response, nb+1 );
      break;
    default:
      nl_error( 4, "Invalid command type in dequeue_request" );
  }
  { int n = strlen(cur_req->request) - 1;
    nl_error( -2, "Dequeued: '%*.*s'", n, n, cur_req->request ); 
  }
  cur_req = NULL;
  if ( ++sbdrq_head >= SUBBUS_MAX_REQUESTS )
    sbdrq_head = 0;
  if (rv == -1)
    nl_error(2, "Error from MsgReply: %s",
      strerror(errno) );
}

/* process_response() reviews the response in
   the buffer to determine if it is a suitable
   response to the current request. If so, it
   is returned to the requester.
   process_response() is not responsible for
   advancing to the next request, but it is
   responsible for dequeuing the current
   request if it has been completed.
 */
#define RESP_OK 0
#define RESP_UNRECK 1
#define RESP_UNEXP 2
#define RESP_INV 3
#define RESP_INTR 4

static void process_interrupt(char *resp, int nb ) {
}

/** ###
 */
static void process_response( char *buf, int nb ) {
  int status = RESP_OK;
  char curcmd = '\0';
  nl_assert( nb > 0 );
  if ( cur_req ) {
    curcmd = cur_req->request[0];
    switch ( buf[0] ) {
      case 'R':
      case 'r':
        if ( curcmd != 'R' ) status = RESP_UNEXP;
        else if ( nb != 5 ) status = RESP_INV;
        else {
          int i;
          for (i = 1; i < 5; i++) {
            if ( ! isxdigit(buf[i]))
              status = RESP_INV;
          }
        }
        break;
      case 'W':
      case 'w':
        if ( curcmd != 'W' ) status = RESP_UNEXP;
        else if (nb != 1) status = RESP_INV;
        break;
      case 'S':
      case 'C':
        if ( curcmd != buf[0] ) status = RESP_UNEXP;
        else if ( ( buf[1] != '0' && buf[1] != '1' ) || nb != 2 )
          status = RESP_INV;
        break;
      case 'V':
        if ( curcmd != 'V' ) status = RESP_UNEXP;
        break;
      case '0':
        if ( curcmd != '\n' ) status = RESP_UNEXP;
        break;
      case 'I':
        status = RESP_INTR; break;
      default:
        status = RESP_UNRECK; break;
    }
  } else {
    switch ( buf[0] ) {
      case 'I':
        status = RESP_INTR; break;
      default:
              status = RESP_UNEXP; break;
    }
  }
  switch (status) {
    case RESP_OK:
      nl_error( -2, "Response: '%s'", buf );
      dequeue_request(buf, nb);
      break;
    case RESP_INTR:
      process_interrupt(buf, nb);
      break;
    case RESP_UNRECK:
      nl_error( 2, "Unreckognized response: '%s'", buf );
      break;
    case RESP_UNEXP:
      nl_error( 2, "Unexpected response: '%s'", buf );
      break;
    case RESP_INV:
      nl_error( 2, "Invalid response: '%s'", buf );
      break;
    default:
      nl_error( 4, "Invalid status: %d", status );
  }
  // we won't dequeue on error: wait for timeout to handle that
  // that's because we don't know the invalid response was
  // to the current request. It could be noise, or an invalid
  // interrupt response for something.
}

/* sb_read_usb() reads data from the serusb device and
   decides what to do with it. If it satisfies the current
   request, we reply to the caller.
   ###
 */
static void sb_read_usb(void) {
  do {
    int nb, nbr;

    nbr = SB_SERUSB_MAX_REQUEST - sb_ibuf_idx;
    nl_assert(nbr > 0 && nbr <= SB_SERUSB_MAX_REQUEST);
    nb = read(sb_fd, &sb_ibuf[sb_ibuf_idx], nbr);
    if ( nb < 0 ) {
      if (errno == EAGAIN) nb = 0;
      else
        nl_error( 3, "Error on read: %s", strerror(errno));
    }
    if ( nb > 0 ) ++n_reads;
    nl_assert(nb >= 0 && nb <= nbr);
    // Check to see if we have a complete response
    while ( nb > 0 ) {
      if ( sb_ibuf[sb_ibuf_idx] == '\n' ) {
        sb_ibuf[sb_ibuf_idx] = '\0';
        process_response(sb_ibuf, sb_ibuf_idx);
        if (--nb > 0) {
          memmove( sb_ibuf, &sb_ibuf[sb_ibuf_idx+1], nb );
          sb_ibuf_idx = nb;
          // do not issue any pending request
          // in order to preserve causality
        } else {
          sb_ibuf_idx = 0;
          process_request();
        }
      } else {
        ++sb_ibuf_idx;
        --nb;
      }
    }
    if ( sb_ibuf_idx > 0 ) ++n_part_reads;
  } while ( sb_data_arm() & _NOTIFY_COND_INPUT );
}

/* sb_data_ready() is a thin wrapper for sb_read_usb()
   which is invoked from dispatch via pulse_attach().
 */
static int sb_data_ready( message_context_t * ctp, int code,
        unsigned flags, void * handle ) {
  sb_read_usb();
  return 0;
}

static void init_serusb(dispatch_t *dpp, int ionotify_pulse) {
  sb_fd = open("/dev/serusb2", O_RDWR | O_NONBLOCK);
  if (sb_fd == -1)
    nl_error(3,"Error opening USB subbus: %s", strerror(errno));
  /* flush anything in the input buffer */
  { int n;
    char tbuf[256];
    do {
      n = read(sb_fd, tbuf, 256);
      if ( n == -1) {
        if (errno == EAGAIN) break;
        else nl_error( 3, "Error trying to clear ibuf: %s",
          strerror(errno));
      }
    } while (n < 256);
  }
  ionotify_event.sigev_notify = SIGEV_PULSE;
  ionotify_event.sigev_code = ionotify_pulse;
  ionotify_event.sigev_priority = getprio(0);
  ionotify_event.sigev_value.sival_int = 0;
  ionotify_event.sigev_coid =
    message_connect(dpp, MSG_FLAG_SIDE_CHANNEL);
  if ( ionotify_event.sigev_coid == -1 )
    nl_error(3, "Could not connect to our channel: %s",
      strerror(errno));

  /* now arm for input */
  sb_read_usb();
}

/**
 This is where we serialize the request
 The basic sanity of the incoming message has been
 checked by subbus_io_msg before it gets here,
 so we can at least assume that the message was
 big enough to include the specified message type,
 and that the message type is defined.
 */
void incoming_sbreq( int rcvid, subbusd_req_t *req ) {
  char sreq[SB_SERUSB_MAX_REQ];
  switch ( req->sbhdr.command ) {
    case SBC_READACK:
      snprintf( sreq, SB_SERUSB_MAX_REQ, "R%04X\n",
	req->data.d1.address );
      break;
    case SBC_WRITEACK:
      snprintf( sreq, SB_SERUSB_MAX_REQ, "W%04X:%04X\n",
	req->data.d0.address, req->data.d0.data );
      break;
    case SBC_SETCMDENBL:
      snprintf( sreq, SB_SERUSB_MAX_REQ, "C%c\n",
	req->data.d1.data ? '1' : '0');
      break;
    case SBC_SETCMDSTRB:
      snprintf( sreq, SB_SERUSB_MAX_REQ, "S%c\n",
	req->data.d1.data ? '1' : '0');
      break;
    case SBC_SETFAIL:
      snprintf( sreq, SB_SERUSB_MAX_REQ, "F%04X\n",
	req->data.d1.data );
      break;
    case SBC_READSW:
      strcpy( sreq, "D\n" ); break;
    case SBC_READFAIL:
      strcpy( sreq, "F\n" ); break;
    case SBC_GETCAPS:
      strcpy( sreq, "V\n" ); break;
    case SBC_TICK:
      strcpy( sreq, "T\n" ); break;
    case SBC_DISARM:
      strcpy( sreq, "A\n" ); break;
    case SBC_INTATT:
      int_attach(req, sreq);
      break;
    case SBC_INTDET:
      int_detach(req, sreq);
      break;
    default:
      nl_error(4, "Undefined command in incoming_sbreq!" );
  }
  enqueue_sbreq( SBDR_TYPE_CLIENT, rcvid, sreq );
}

void init_subbus(dispatch_t *dpp ) {
  /* Setup ionotify pulse handler */
  int ionotify_pulse =
    pulse_attach(dpp, MSG_FLAG_ALLOC_PULSE, 0, sb_data_ready, NULL);
  init_serusb(dpp, ionotify_pulse);

  /* Setup timer pulse handler */
  // pulse_attach();

  /* Enqueue initialization requests */
  ### enqueue_sbreq( SBDR_TYPE_INTERNAL, 0, "\n" );
  ### enqueue_sbreq( SBDR_TYPE_INTERNAL, 0, "V\n" );
}

void shutdown_subbus(void) {
  nl_error( 0, "%d writes, %d reads, %d partial reads",
    n_writes, n_reads, n_part_reads );
}
