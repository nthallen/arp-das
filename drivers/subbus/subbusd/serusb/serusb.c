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

static subbusd_cap_t subbus_caps;

// Transmits a request if the currently queued
// request has not been transmitted.
static void process_request(void) {
  int cmdlen, n;
  int no_response = 0;
  sbd_request_t *sbr;
  while ( sbdrq_head != sbdrq_tail && cur_req == NULL ) {
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
	  case 'T':
	  case 'A':
	    no_response = 1; break;
	  case 'R':
	  case 'W':
	  case 'V':
	  case 'S':
	  case 'C':
	  case 'B':
	  case 'I':
	  case 'D':
	  case 'F':
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
    if ( no_response )
      dequeue_request( "", 0 ); // ret_status = SBS_OK, ret_type = SBRT_NONE
  }
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
  /* If the queue was empty, process this first request */
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

/**
 * Sends the response to the client (if any) and
 * removes it from the queue. Does not initiate
 * processing of the next command.
 * Current assumption:
 *    n_args maps 1:1 onto SBRT_ codes
 *    n_args == 3 is only for 'V' request/response
 */
static void dequeue_request( signed short status, int n_args,
  unsigned short arg0, unsigned short arg1, char *s ) {
  int rv, rsize;
  subbus_rep_t rep;

  nl_assert( cur_req != NULL);
  rep.hdr.status = status;
  switch (n_args) {
    case 0:
      rep.hdr.ret_type = SBRT_NONE;
      rsize = sizeof(subbusd_rep_hdr_t);
      break;
    case 1:
      rep.hdr.ret_type = SBRT_US;
      rep.data.value = arg0;
      rsize = sizeof(subbusd_rep_hdr_t)+sizeof(unsigned short);
      break;
    case 3:
      nl_assert( cur_req->request[0] == 'V' );
      switch ( cur_req->type ) {
	case SBDR_TYPE_INTERNAL:
	  nl_error( 0, "Features: %d:%03X Version: %s",
	    arg0, arg1, s);
	  break;
	case SBDR_TYPE_CLIENT:
	  rep.hdr.ret_type = SBRT_CAP;
	  rep.data.capabilities.subfunc = arg0;
	  rep.data.capabilities.features = arg1;
	  strncpy(rep.data.capabilities.name, s, SUBBUS_NAME_MAX );
	  rsize = sizeof(subbusd_rep_t);
	  break;
	default: 
	  break; // picked up and reported below
      }
      break;
    case 2:
      nl_error( 4, "Invalid n_args in dqueue_request" );
  }
  switch( cur_req->type ) {
    case SBDR_TYPE_INTERNAL:
      rv = 0;
      break;
    case SBDR_TYPE_CLIENT:
      rv = MsgReply( cur_req->rcvid, rsize, rep, rsize );
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

static void process_interrupt(char *resp, int nb ) {
}

/* process_response() reviews the response in
   the buffer to determine if it is a suitable
   response to the current request. If so, it
   is returned to the requester.
   process_response() is not responsible for
   advancing to the next request, but it is
   responsible for dequeuing the current
   request if it has been completed.

   The string in resp is guaranteed to have had
   a newline at the end, which was replaced with
   a NUL, so we are guaranteed to have a NUL-
   terminated string.
 */
#define RESP_OK 0
#define RESP_UNRECK 1 /* Unrecognized code */
#define RESP_UNEXP 2 /* Unexpected code */
#define RESP_INV 3 /* Invalid response syntax */
#define RESP_INTR 4 /* Interrupt code */
#define RESP_ERR 5 /* Error from serusb */

/** parses the ASCII response from serusb
 * and prepares it for dequeue_request()
 */
static void process_response( char *buf ) {
  int status = RESP_OK;
  unsigned short arg0, arg1;
  int n_args = 0;
  char *s = buf;
  char resp_code = *s++;
  char exp_req = '\0';
  int exp_args = 0;
  nl_assert( resp_code != '\0' );
  if (read_hex( &s, &arg0 )) {
    ++n_args;
    if (*s == ':' && read_hex( &++s, &arg1 ) ) {
      ++n_args;
      if ( *s == ':' ) {
	++s; // points to name
	++n_args;
      } else {
	status = RESP_INV;
      }
    } else if ( *s != '\0' ) {
      status = RESP_INV;
    }
  } else if ( *s != '\0' ) {
    status = RESP_INV;
  }
  // Check response for self-consistency
  // Check that response is appropriate for request
  switch (resp_code) {
    case 'R':
    case 'r':
      exp_req = 'R';
      exp_args = 1;
      break;
    case 'W':
    case 'w':
      exp_req = 'W';
      exp_args = 0;
      break;
    case 'S':
    case 'C':
      exp_req = resp_code;
      exp_args = 1;
      break;
    case 'V':
      exp_req = 'V';
      exp_args = 3;
      break;
    case 'I':
      status = RESP_INTR;
      exp_req = '\0';
      exp_args = 1;
      break;
    case 'i':
    case 'u':
      exp_req = 'i';
      exp_args = 1;
      break;
    case '0':
      exp_req = '\n';
      exp_args = 0;
      break;
    case 'D':
    case 'F':
      exp_req = resp_code;
      exp_args = 1;
      break;
    case 'E':
      status = RESP_ERR;
      exp_req = '\0';
      exp_args = 1;
      break;
    default:
      status = RESP_UNRECK;
      break;
  }
  switch (status) {
    case RESP_OK:
      if ( cur_req == NULL || cur_req->request[0] != exp_req) {
	status = RESP_UNEXP;
	break;
      } // fall through
    case RESP_INTR:
    case RESP_ERR:
      if (n_args != exp_args)
	status = RESP_INV;
      break;
  }
  switch (status) {
    case RESP_OK:
      dequeue_request(SBS_OK, n_args, arg0, arg1, s);
      break;
    case RESP_INTR:
      process_interrupt(arg0);
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
        process_response(sb_ibuf);
        if (--nb > 0) {
          memmove( sb_ibuf, &sb_ibuf[sb_ibuf_idx+1], nb );
          sb_ibuf_idx = nb;
          // do not issue any pending request
          // in order to preserve causality
        } else {
          sb_ibuf_idx = 0;
          /* I'm assuming the previous process_response()
             managed to dequeue the current request,
             but process_request() will quietly return
             if that is not the case. */
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
  enqueue_sbreq( SBDR_TYPE_INTERNAL, 0, "\n" );
  enqueue_sbreq( SBDR_TYPE_INTERNAL, 0, "V\n" );
}

void shutdown_subbus(void) {
  nl_error( 0, "%d writes, %d reads, %d partial reads",
    n_writes, n_reads, n_part_reads );
}