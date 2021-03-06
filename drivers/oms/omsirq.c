#include <stdio.h>
#include <ctype.h>
#include <hw/inout.h>
#include "omsint.h"
#include "nortlib.h"
#include "nl_assert.h"

readreq *current_req;

reqqueue *pending_queue;
charqueue *output_queue;
volatile int tbe_enabled = 0;
volatile int irq_seen = 0;
volatile int irq_still_asserted = 0;
static int pq_mode = PQ_MODE_IDLE;

extern void handle_char( char c );

/*
  I cannot find an interrupt reset in the OMS documentation,
  so I am assuming that I need to make sure all requests have
  been served before I return or the appropriate interrupt
  should be disabled.
*/
void service_int( void ) {
  irq_seen = 1;
  for (;;) {
    unsigned char status = in8(PC68_STATUS);
    if ( (status & PC68_INIT_S) == 0 ) {
      if (status & PC68_IBF_S ) {
        int c = in8( PC68_DATA );
        handle_char(c);
        // if ( handle_char(c) ) handle_recv_data();
      } else if ( tbe_enabled && ( status & PC68_TBE_S ) ) {
        int c = dequeue_char( output_queue );
        if ( c ) {
          out8( PC68_DATA, c );
        } else {
          /* disable TBE Interrupt, leave IRQ_E and IBF_E */
          tbe_enabled = 0;
          out8( PC68_CONTROL, 0xA0 );
        }
      } else {
        if ( status & PC68_IRQ_S ) irq_still_asserted++;
        break;
      }
    }
  }
}

/* handle_char takes a char at a time and looks for
   status : error_char | status_string
   error_char : '#' | '$' | '@' | '!'
   status_string : <lf><cr>data<lf><cr>
   I'm guessing that an error_char at any time will
   reset us to the beginning state. I'm also assuming
   that anything non-printable except for <cr><lf> is
   invalid.
   
   state 0
         <lf> => state 1
   state 4
         <lf> => state 1
         <cr> => state 4
   state 1
         <cr> => state 2
         <lf> => state 1 (error)
   state 2
         This is where we enqueue printable characters
         <cr> => state 2
         <lf> => state 3
   state 3
         <cr> => state 0
         <lf> => state 1 (error)
*/
static int ibuf_idx = 0;
void handle_char( char c ) {
  switch (pq_mode) {
    case PQ_MODE_IDLE:
      // nl_error(1, "Unexpected character %d in handle_char", c);
      break;
    case PQ_MODE_SENDING:
      ibuf_idx = 0;
      current_req->ibuf[ibuf_idx++] = c;
      pq_mode = PQ_MODE_RECEIVING;
      set_oms_timeout( OMS_PAUSE_TIMEOUT );
      break;
    case PQ_MODE_RECEIVING:
      if ( ibuf_idx < IBUF_SIZE )
        current_req->ibuf[ibuf_idx++] = c;
      set_oms_timeout( OMS_PAUSE_TIMEOUT );
      break;
    default:
      nl_error(4, "Invalid pq_mode value in handle_char(): %d", pq_mode);
  }
}

// int handle_char( char c ) {
  // if ( current_req == NULL ) {
    // current_req = dequeue_req( pending_queue );
    // if (current_req)
      // current_req->n_req_togo = current_req->n_req;
  // }
  // if ( current_req != NULL ) {
    // switch ( c ) {
      // case '#':
      // case '$':
      // case '@':
      // case '!':
        // /* Except in the case where an invalid read request
        // results in a command error ('#'), these should not
        // affect queued read requests. Perhaps in that case,
        // the error should be written into the buffer of the
        // top queued read request and reported. */
        // nl_error(1, "Read error code '%c' from OMS", c );
        // hc_state = 0;
        // return 0;
      // case '\n':
      // case '\r':
        // break;
      // default:
        // if ( isprint(c) ) {
          // if ( hc_state != 2 ) {
            // ibuf_idx = 0;
            // /* unexpected character */
            // nl_error( -2, "Unexpect printable char '%c' in hc_state %d",
              // c, hc_state );
            // hc_state = 2;
          // }
        // } else {
          // /* unexpected unprintable char, discarded */
          // nl_error(-2,
            // "Unexpected non-printable char in state %d: %d",
            // hc_state, c );
          // hc_state = 0;
          // return 0;
        // }
        // break;
    // }
    // switch ( hc_state ) {
      // case 0:
        // if ( c == '\n' ) {
          // hc_state = 1;
          // ibuf_idx = 0;
          // return 0;
        // }
        // break;
      // case 4: /* 4 is like 0, but doesn't reset ibuf_idx */
        // if ( c == '\n' ) {
          // hc_state = 1;
          // return 0;
        // } else if ( c == '\r' )
          // return 0;
        // break;
      // case 1:
        // if ( c == '\r' ) { hc_state = 2; return 0; }
        // break;
      // case 2:
        // if ( c == '\r' ) {
          // nl_error(-2,"Discarding CR in hc_state 2" );
          // /* ignore */
        // } else if ( c == '\n' ) {
          // current_req->n_req_togo--;
          // current_req->ibuf[ibuf_idx++] =
            // current_req->n_req_togo ? ';' : '\0';
          // hc_state = 3;
        // } else current_req->ibuf[ibuf_idx++] = c;
        // return 0;
      // case 3:
        // if ( c == '\r' ) {
          // hc_state = 0;
          // if ( current_req->n_req_togo == 0 ) {
            // enqueue_req( satisfied_queue, current_req );
            // current_req = NULL;
            // return 1;
          // } else hc_state = 4;
          // return 0;
        // }
        // break;
      // default:
        // /* invalid hc_state! reset it */
        // hc_state = 0;
        // return 0;
    // }
    // /* invalid character... */
    // hc_state = 0;
  // } else {
        // /* else no pending requests, so character is discarded */
    // nl_error( -2, "Unexpected character code %d", c );
  // }
  // return 0;
// }

/* queue semantics:
  if head==tail queue is empty, else
  head points to first in queue
  tail points to next free space
  
  return non-zero if queue is full
*/
int enqueue_req( reqqueue *queue, readreq *req ) {
  int next = queue->tail+1;
  if ( next >= queue->size ) next = 0;
  if ( next == queue->head ) return 1;
  queue->buf[queue->tail] = req;
  queue->tail = next;
  return 0;
}

readreq *dequeue_req( reqqueue *queue ) {
  readreq *req;
  int next;
  if ( queue->head == queue->tail ) return NULL;
  req = queue->buf[queue->head];
  next = queue->head + 1;
  if ( next >= queue->size ) next = 0;
  queue->head = next;
  return req;
}

/* return 1 if the queue is full */
int enqueue_char( charqueue *queue, char qchar ) {
  int next;
  
  next = queue->tail+1;
  if ( next >= queue->size ) next = 0;
  if ( next == queue->head ) return 1;
  queue->buf[queue->tail] = qchar;
  queue->tail = next;
  if ( ! tbe_enabled ) {
    /* _disable(); */
    out8( PC68_CONTROL, 0xE0 ); /* enable TBE interrupt */
    tbe_enabled = 1;
    service_int();
  }
  return 0;
}

static void oms_queue_output( char *cmd ) {
  while ( *cmd != '\0' ) enqueue_char(output_queue, *cmd++);
}

char dequeue_char( charqueue *queue ) {
  char qchar;
  int next;
  if ( queue->head == queue->tail ) return '\0';
  qchar = queue->buf[queue->head];
  next = queue->head + 1;
  if ( next >= queue->size ) next = 0;
  queue->head = next;
  return qchar;
}

/*
  queue of pending read requests
        enqueued by oms_read
        dequeued by ISR
  queue of satisfied read requests
        enqueued by ISR
        dequeued by proxy service routine
  queue of outbound characters
        enqueued by oms_write
        dequeued by ISR

  read request {
        code for what to do with the result {
          How many requests?
          What should I do with it {
                Send to TM
                Log via msg()
                Reply to sender
          }
        }
  }
*/


void pq_check(void) {
  if (pq_mode != PQ_MODE_IDLE) return;
  nl_assert(current_req == NULL);
  current_req = dequeue_req( pending_queue );
  if (current_req == NULL) return;
  pq_mode = PQ_MODE_SENDING;
  oms_queue_output(current_req->cmd);
  if (current_req->req_type == OMSREQ_NO_RESPONSE) {
    pq_recycle();
  } else {
    set_oms_timeout( OMS_INITIAL_TIMEOUT );
  }
}

/**
 * Dispose of current_req
 */
void pq_recycle(void) {
  switch (current_req->req_type) {
    case OMSREQ_IGNORE:
    case OMSREQ_LOG:
    case OMSREQ_NO_RESPONSE:
      enqueue_req( free_queue, current_req );
      break;
    case OMSREQ_PROCESSTM:
      Col_send( current_req->u.tm.tmid );
      break;
    default:
      nl_error(4, "Invalid request type: %d",
		current_req->req_type );
  }
  current_req = NULL;
  pq_mode = PQ_MODE_IDLE;
  pq_check();
}

void pq_timeout(void) {
  switch( pq_mode ) {
    case PQ_MODE_IDLE: return;
    case PQ_MODE_SENDING:
      nl_assert(current_req != NULL);
      switch (current_req->req_type) {
        case OMSREQ_IGNORE: break;
        case OMSREQ_PROCESSTM:
          nl_error(2, "Timeout waiting for TM data" );
          break;
        case OMSREQ_LOG:
          nl_error(2, "Timeout waiting for status request: '%s'",
            current_req->cmd );
          break;
        default:
          nl_error(4, "Invalid request type: %d",
		    current_req->req_type );
      }
      pq_recycle();
      break;
    case PQ_MODE_RECEIVING:
      if (ibuf_idx >= IBUF_SIZE) {
        current_req->ibuf[IBUF_SIZE-1] = '\0';
        nl_error( 2, "Input buffer overflow on command '%s': '%s'",
          current_req->cmd, quote_np(current_req->ibuf) );
      } else {
        current_req->ibuf[ibuf_idx] = '\0';
      }
      handle_recv_data();
      break;
    default:
      nl_error(4, "Invalid pq_mode value in pq_timeout: %d", pq_mode );
  }
}
