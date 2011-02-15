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
static sbd_request_t sbdrq[SUBBUSD_MAX_REQUESTS];
static sbd_request_t *cur_req;
static unsigned int sbdrq_head = 0, sbdrq_tail = 0;
static int sb_fd;
static struct sigevent ionotify_event;
static timer_t timeout_timer;
static struct itimerspec timeout_enable, timeout_disable;
static int n_timeouts = 0;

static int n_writes = 0;
static int n_reads = 0;
static int n_part_reads = 0;
static int n_compound_reads = 0;

static void dequeue_request( signed short status, int n_args,
  unsigned short arg0, unsigned short arg1, char *s );

static void set_timeout( int enable ) {
  if ( timer_settime( timeout_timer, 0,
          enable ? &timeout_enable : &timeout_disable,
          NULL ) == -1 ) {
    nl_error( 4, "Error setting timer: %s",
      strerror(errno) );
  }
}

#define ESC_BUF_SIZE 80
static char *ascii_escape(const char *ibuf) {
  static char ebuf[ESC_BUF_SIZE+3];
  int ix = 0, ox = 0;
  while (ibuf[ix] != '\0' && ox < ESC_BUF_SIZE ) {
    char c = ibuf[ix++];
    if ( isprint(c) ) {
      ebuf[ox++] = c;
    } else {
      switch ( c ) {
        case '\n':
          ebuf[ox++] = '\\';
          ebuf[ox++] = 'n';
          break;
        case '\r':
          ebuf[ox++] = '\\';
          ebuf[ox++] = 'r';
          break;
        case '\t':
          ebuf[ox++] = '\\';
          ebuf[ox++] = 't';
          break;
        default:
          ox += snprintf( ebuf+ox, 4, "\\x%02x", c);
          break;
      }
    }
  }
  ebuf[ox] = '\0';
  return ebuf;
}

// Transmits a request if the currently queued
// request has not been transmitted.
static void process_request(void) {
  int cmdlen, n;
  int no_response = 0;
  sbd_request_t *sbr;
  while ( sbdrq_head != sbdrq_tail && cur_req == NULL ) {
    nl_assert( sbdrq_head < SUBBUSD_MAX_REQUESTS );
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
            no_response = 1; break;
          case 'A':
          case 'R':
          case 'W':
          case 'V':
          case 'S':
          case 'C':
          case 'B':
          case 'i':
          case 'u':
          case 'D':
          case 'F':
          case 'f':
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
      dequeue_request( SBS_OK, 0, 0, 0, "" );
    else set_timeout(1);
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
  if ( new_tail >= SUBBUSD_MAX_REQUESTS ) new_tail = 0;
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
  subbusd_rep_t rep;

  nl_assert( cur_req != NULL);
  set_timeout(0);
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
          rsize = sizeof(subbusd_rep_hdr_t) + sizeof(subbusd_cap_t);
          break;
        default: 
          break; // picked up and reported below
      }
      break;
    case 2:
      nl_error( 4, "Invalid n_args in dequeue_request" );
  }
  switch( cur_req->type ) {
    case SBDR_TYPE_INTERNAL:
      rv = 0;
      break;
    case SBDR_TYPE_CLIENT:
      rv = MsgReply( cur_req->rcvid, rsize, &rep, rsize );
      break;
    default:
      nl_error( 4, "Invalid command type in dequeue_request" );
  }
  { int n = strlen(cur_req->request) - 1;
    nl_error( -2, "Dequeued: '%*.*s'", n, n, cur_req->request ); 
  }
  cur_req = NULL;
  if ( ++sbdrq_head >= SUBBUSD_MAX_REQUESTS )
    sbdrq_head = 0;
  if (rv == -1)
    nl_error(2, "Error from MsgReply: %s",
      strerror(errno) );
  process_request(); // if one is pending...
}

static void process_interrupt( unsigned int nb ) {
  card_def *cd;
  unsigned short addr;
  unsigned int bn;
  char sreq[8];
  
  for ( cd = carddefs; cd != NULL && cd->bitno != nb; cd = cd->next ) {}
  if ( cd != NULL ) {
    int rv = MsgDeliverEvent( cd->owner, &cd->event );
    if ( rv == -1 ) {
      switch (errno) {
        case EBADF:
        case ESRCH:
          nl_error( 1,
            "Process attached to '%s' interrupt not found",
            cd->cardID );
          rv = expint_detach( cd->owner, cd->cardID, &addr, &bn );
          nl_assert( rv == EOK );
          nl_assert( nb == bn );
          snprintf( sreq, 8, "u%X\n", addr );
          enqueue_sbreq( SBDR_TYPE_INTERNAL, 0, sreq );
          break;
        default:
          nl_error( 4, "Unexpected error %d from MsgDeliverEvent: %s",
              errno, strerror(errno));
      }
    }
  } else {
    nl_error( 1, "Unexpected interrupt #%d", nb );
    // We don't know the address, so can't disable it.
    // Probably a late hit.
  }
}


/**
 Parses the input string for a hexadecimal integer.
 @return zero on failure.
 */
static int read_hex( char **sp, unsigned short *arg ) {
  char *s = *sp;
  unsigned short val = 0;
  if ( ! isxdigit(*s) )
    return 0;
  while ( isxdigit(*s) ) {
    val *= 16;
    if ( isdigit(*s) )
      val += *s - '0';
    else
      val += tolower(*s) - 'a' + 10;
    ++s;
  }
  *arg = val;
  *sp = s;
  return 1;
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
#define RESP_UNREC 1 /* Unrecognized code */
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
  signed short sbs_ok_status = SBS_OK;
  int n_args = 0;
  char *s = buf;
  char resp_code = *s++;
  char exp_req = '\0';
  int exp_args = 0;
  if ( resp_code != '\0' ) {
    if (read_hex( &s, &arg0 )) {
      ++n_args;
      if (*s == ':') {
        ++s;
        if ( read_hex( &s, &arg1 ) ) {
          ++n_args;
          if ( *s == ':' ) {
            ++s; // points to name
            ++n_args;
          } else {
            status = RESP_INV;
          }
        } else status = RESP_INV;
      } else if ( *s != '\0' ) {
        status = RESP_INV;
      }
    } else if ( *s != '\0' ) {
      status = RESP_INV;
    }
  }
  // Check response for self-consistency
  // Check that response is appropriate for request
  switch (resp_code) {
    case 'R':
      exp_req = 'R';
      exp_args = 1;
      sbs_ok_status = SBS_ACK;
      break;
    case 'r':
      exp_req = 'R';
      exp_args = 1;
      sbs_ok_status = SBS_NOACK;
      break;
    case 'W':
      exp_req = 'W';
      exp_args = 0;
      sbs_ok_status = SBS_ACK;
      break;
    case 'w':
      exp_req = 'W';
      exp_args = 0;
      sbs_ok_status = SBS_NOACK;
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
    case 'A':
    case 'B':
    case 'S':
    case 'C':
    case 'F':
      exp_req = resp_code;
      exp_args = 0;
      break;
    case '0':
      exp_req = '\n';
      exp_args = 0;
      break;
    case 'D':
    case 'f':
    case 'i':
    case 'u':
      exp_req = resp_code;
      exp_args = 1;
      break;
    case 'E':
      status = RESP_ERR;
      exp_req = '\0';
      exp_args = 1;
      break;
    default:
      status = RESP_UNREC;
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
      dequeue_request(sbs_ok_status, n_args, arg0, arg1, s);
      break;
    case RESP_INTR:
      process_interrupt(arg0);
      break;
    case RESP_UNREC:
      nl_error( 2, "Unrecognized response: '%s'", ascii_escape(buf) );
      break;
    case RESP_UNEXP:
      nl_error( 2, "Unexpected response: '%s'", ascii_escape(buf) );
      break;
    case RESP_INV:
      nl_error( 2, "Invalid response: '%s'", ascii_escape(buf) );
      break;
    case RESP_ERR:
      nl_error( 2, "Error code %d from DACS", ascii_escape(buf) );
      break;
    default:
      nl_error( 4, "Invalid status: %d", status );
  }
  switch (status) {
    case RESP_OK:
    case RESP_INTR: break;
    default:
      if ( cur_req )
        nl_error( 2, "Current request was: '%s'",
            ascii_escape(cur_req->request) );
      else
        nl_error( 2, "No current request" );
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
          sb_ibuf_idx = 0;
          ++n_compound_reads;
          // do not issue any pending request
          // in order to preserve causality
        } else {
          sb_ibuf_idx = 0;
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

static int sb_timeout( message_context_t * ctp, int code,
        unsigned flags, void * handle ) {
  if ( ++n_timeouts > 1 ) {
    n_timeouts = 0;
    if ( cur_req != NULL ) {
      nl_error( 1, "%sUSB request '%c' timed out",
        (cur_req->type == SBDR_TYPE_INTERNAL) ? "Internal " : "",
        cur_req->request[0] );
      dequeue_request( -ETIMEDOUT, 0, 0, 0, "" );
    }
  }
  return 0;
}

static void init_serusb(dispatch_t *dpp, int ionotify_pulse,
                          int timeout_pulse) {
  struct sigevent timeout_event;
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
  timeout_event.sigev_notify = SIGEV_PULSE;
  timeout_event.sigev_code = timeout_pulse;
  timeout_event.sigev_priority = getprio(0);
  timeout_event.sigev_value.sival_int = 0;
  timeout_event.sigev_coid = ionotify_event.sigev_coid;
  if ( timer_create( CLOCK_REALTIME, &timeout_event,
          &timeout_timer ) == -1 )
    nl_error(3, "Could not create timer: %s",
      strerror(errno));
  timeout_enable.it_value.tv_sec = 0;
  timeout_enable.it_value.tv_nsec = 100000000L;
  timeout_enable.it_interval.tv_sec = 0;
  timeout_enable.it_interval.tv_nsec = 100000000L;
  timeout_disable.it_value.tv_sec = 0;
  timeout_disable.it_value.tv_nsec = 0;
  timeout_disable.it_interval.tv_sec = 0;
  timeout_disable.it_interval.tv_nsec = 0;

  /* now arm for input */
  sb_read_usb();
}

static void ErrorReply( int rcvid, int rv ) {
  subbusd_rep_hdr_t rep;
  nl_assert( rv > 0 );
  rep.status = -rv;
  rep.ret_type = SBRT_NONE;
  rv = MsgReply( rcvid, sizeof(rep), &rep, sizeof(rep) );
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
  char sreq[SB_SERUSB_MAX_REQUEST];
  subbusd_rep_t rep;
  int rv;
  
  switch ( req->sbhdr.command ) {
    case SBC_READCACHE:
      rep.hdr.status =
        (sb_read_cache(req->data.d1.data, &rep.data.value) < 0) ?
        SBS_ACK : SBS_NOACK;
      rep.hdr.ret_type = SBRT_US;
      rsize =
        sizeof(subbusd_rep_hdr_t) + sizeof(unsigned short);    
      rv = MsgReply( rcvid, rsize, &rep, rsize );
      return;
    case SBC_READACK:
      snprintf( sreq, SB_SERUSB_MAX_REQUEST, "R%04X\n",
        req->data.d1.data );
      break;
    case SBC_WRITECACHE:
      rv = sb_write_cache(req->data.d0.address, req->data.d0.data);
      if (rv != 1) {
        rep.hdr.ret_type = SBRT_NONE;
        rep.hdr.status = (rv == 0) ? SBS_ACK : SBS_NOACK;
        rsize = sizeof(subbusd_rep_hdr_t);
        rv = MsgReply( rcvid, rsize, &rep, rsize );
        return;
      }
      /* else fall through */
    case SBC_WRITEACK:
      snprintf( sreq, SB_SERUSB_MAX_REQUEST, "W%04X:%04X\n",
        req->data.d0.address, req->data.d0.data );
      break;
    case SBC_SETCMDENBL:
      snprintf( sreq, SB_SERUSB_MAX_REQUEST, "C%c\n",
        req->data.d1.data ? '1' : '0');
      break;
    case SBC_SETCMDSTRB:
      snprintf( sreq, SB_SERUSB_MAX_REQUEST, "S%c\n",
        req->data.d1.data ? '1' : '0');
      break;
    case SBC_SETFAIL:
      snprintf( sreq, SB_SERUSB_MAX_REQUEST, "F%04X\n",
        req->data.d1.data );
      break;
    case SBC_READSW:
      strcpy( sreq, "D\n" ); break;
    case SBC_READFAIL:
      strcpy( sreq, "f\n" ); break;
    case SBC_GETCAPS:
      strcpy( sreq, "V\n" ); break;
    case SBC_TICK:
      strcpy( sreq, "T\n" ); break;
    case SBC_DISARM:
      strcpy( sreq, "A\n" ); break;
    case SBC_INTATT:
      rv = int_attach(rcvid, req, sreq);
      if (rv != EOK) {
        ErrorReply(rcvid, rv);
        return; // i.e. don't enqueue
      }
      break;
    case SBC_INTDET:
      rv = int_detach(rcvid, req, sreq);
      if (rv != EOK) {
        ErrorReply(rcvid, rv);
        return; // i.e. don't enqueue
      }
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
  int timer_pulse =
    pulse_attach(dpp, MSG_FLAG_ALLOC_PULSE, 0, sb_timeout, NULL);
  init_serusb(dpp, ionotify_pulse, timer_pulse);

  /* Setup timer pulse handler */
  // pulse_attach();

  /* Enqueue initialization requests */
  enqueue_sbreq( SBDR_TYPE_INTERNAL, 0, "\n" );
  enqueue_sbreq( SBDR_TYPE_INTERNAL, 0, "V\n" );
}

void shutdown_subbus(void) {
  nl_error( 0, "%d writes, %d reads, %d partial reads, %d compound reads",
    n_writes, n_reads, n_part_reads, n_compound_reads );
}
