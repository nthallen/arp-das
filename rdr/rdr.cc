#include "rdr.h"
#include "nortlib.h"
#include "oui.h"

#define RDR_BUFSIZE 16384

int main( int argc, char **argv ) {
  oui_init_options( argc, argv );
  nl_error(0, "Startup");
  load_tmdac(NULL);
  int nQrows = RDR_BUFSIZE/tmi(nbrow);
  if (nQrows < 2) nQrows = 2;
  Reader rdr(nQrows, nQrows/2, RDR_BUFSIZE );
  initialize the semaphores
  start up threads
  rdr::data_generator.operate();
  nl_error(0, "Shutdown");
}

/**
  The initialization here is a little frustrating because we don't really know what parameters we want until
  we have loaded tm.dac and learn the frame dimensions. Currently load_tmdac is a DC method, so we can't
  invoke it until 
 */
Reader::Reader(int nQrows, int low_water, int bufsize) :
    data_generator(nQrows, low_water), data_client( bufsize, 0, NULL ) {
  it_blocked = 0;
  ot_blocked = 0;
  if ( sem_init( &it_sem, 0, 1) || sem_init( &ot_sem, 0, 1 ) )
    nl_error( 3, "Semaphore initialization failed" );
  int rv = pthread_mutex_init( &dq_mutex, NULL );
  if ( rv )
    nl_error( 3, "Mutex initialization failed: %s",
            strerror(errno));
  init_tm_type();
}

void Reader::lock() {
  int rv = pthread_mutex_lock(&dq_mutex);
  if (rv)
    nl_error( 3, "Mutex lock failed: %s",
            strerror(errno));
}

void Reader::unlock() {
  int rv = pthread_mutex_unlock(&dq_mutex);
  if (rv)
    nl_error( 3, "Mutex unlock failed: %s",
            strerror(errno));
}

void Reader::event(enum dg_event evt) {
  switch (evt) {
    case dg_event_start:
      lock();
      if (ot_blocked) {
        ot_blocked = 0;
        post(&ot_sem);
      }
      unlock();
      break;
    default:
      break;
  }
}

void * input_thread(void *Reader_ptr ) {
  Reader *DGr = (Reader *)Reader_ptr;
  return DGr->input_thread();
}

void *Reader::input_thread() {
}

void *output_thread(void *Reader_ptr ) {
  Reader *DGr = (Reader *)Reader_ptr;
  return DGr->output_thread();
}

void *Reader::output_thread() {
  for (;;) {
    lock();
    if ( quit ) {
      unlock();
      break;
    }
    if ( ! started ) {
      ot_blocked = OT_BLOCKED_STOPPED;
      unlock();
      sem_wait(&ot_sem);
    } else {
      ot_stopped = false;
      if ( regulated ) {
        // timed loop
        for (;;) {
          ot_blocked = OT_BLOCKED_TIME;
          unlock();
          sem_wait(&ot_sem);
          lock();
          int breakout = !started || !regulated;
          unlock();
          if (breakout) break;
          transmit_data(1); // only one row
          if (allocate_rows(NULL) >= dq_low_water) {
            lock();
            if ( it_blocked == IT_BLOCKED_DATA ) {
              it_blocked = 0;
              sem_post(&it_sem);
            }
            unlock();
          }
          lock(); /* needed in the inner loop */
        }
      } else {
        // untimed loop
        for (;;) {
          int breakout = !started || regulated;
          unlock();
          if (breakout) break;
          get_data();
          transmit_data(0);
          lock();
        }
      }
    }
  }
  signal parent thread that we are quitting
}

/** This is the basic operate loop for a simple extraction
 *
 */
void Reader::it_operate() {
  while ( !quit ) {
    // check for quit
    read();
  }
}

void Reader::process_tstamp() {
  if ( have_tstamp &&
       tm_info.t_stmp.mfc_num == msg->body.ts.mfc_num &&
       tm_info.t_stmp.secs == msg->body.ts.secs )
    return; // redundant tstamp (beginning of each new file)
  tm_info.t_stmp = msg->body.ts;
  if (!have_tstamp) {
    data_generator::init(0); // DG initialization
    have_stamp = true;
  } else commit_tstamp( tm_info.t_stmp.mfc_num, tm_info.t_stmp.secs );
}

void Reader::process_data() {
  if ( ! have_tstamp ) {
    nl_error(1, "process_data() without initialization" );
    return;
  }
  tm_data_t3_t *data = &msg->body.data3;
  unsigned char *raw = &data->data[0];
  int n_rows = data->n_rows;
  
}
