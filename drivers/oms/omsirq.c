#include <stdio.h>
#include <ctype.h>
#include <hw/inout.h>
#include "omsint.h"

static readreq *current_req;

reqqueue *pending_queue;
reqqueue *satisfied_queue;
charqueue *output_queue;
volatile int tbe_enabled = 0;
volatile int irq_seen = 0;
volatile int irq_still_asserted = 0;
extern int handle_char( char c );

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
	if ( handle_char(c) ) handle_recv_data();
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
	 <cr> => state 2
	 <lf> => state 3
   state 3
	 <cr> => state 0
	 <lf> => state 1 (error)
*/
static int hc_state = 0;
static int ibuf_idx = 0;
int handle_char( char c ) {
  if ( current_req == NULL ) {
    current_req = dequeue_req( pending_queue );
    current_req->n_req_togo = current_req->n_req;
  }
  if ( current_req != NULL ) {
    switch ( c ) {
      case '#':
      case '$':
      case '@':
      case '!':
	/* Except in the case where an invalid read request
	results in a command error ('#'), these should not
	affect queued read requests. Perhaps in that case,
	the error should be written into the buffer of the
	top queued read request and reported. */
	hc_state = 0;
	return 0;
      case '\n':
      case '\r':
	break;
      default:
	if ( isprint(c) ) {
	  if ( hc_state != 2 ) {
	    ibuf_idx = 0;
	    /* unexpected character */
	    hc_state = 2;
	  }
	} else {
	  /* unexpected unprintable char, discarded */
	  hc_state = 0;
	  return 0;
	}
	break;
    }
    switch ( hc_state ) {
      case 0:
	if ( c == '\n' ) {
	  hc_state = 1;
	  ibuf_idx = 0;
	  return 0;
	}
	break;
      case 4: /* 4 is like 0, but doesn't reset ibuf_idx */
	if ( c == '\n' ) {
	  hc_state = 1;
	  return 0;
	} else if ( c == '\r' )
	  return 0;
	break;
      case 1:
	if ( c == '\r' ) { hc_state = 2; return 0; }
	break;
      case 2:
	if ( c == '\r' ) {
	  /* ignore */
	} else if ( c == '\n' ) {
	  current_req->n_req_togo--;
	  current_req->ibuf[ibuf_idx++] =
	    current_req->n_req_togo ? ';' : '\0';
	  hc_state = 3;
	} else current_req->ibuf[ibuf_idx++] = c;
	return 0;
      case 3:
	if ( c == '\r' ) {
	  hc_state = 0;
	  if ( current_req->n_req_togo == 0 ) {
	    enqueue_req( satisfied_queue, current_req );
	    current_req = NULL;
	    return 1;
	  } else hc_state = 4;
	  return 0;
	}
	break;
      default:
	/* invalid hc_state! reset it */
	hc_state = 0;
	return 0;
    }
    /* invalid character... */
    hc_state = 0;
  } else {
	/* else no pending requests, so character is discarded */
  }
  return 0;
}

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
