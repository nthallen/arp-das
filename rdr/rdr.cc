#include "rdr.h"
#include "nortlib.h"
#include "oui.h"
#include "nl_assert.h"
#include "mlf.h"

#define RDR_BUFSIZE 16384

int main( int argc, char **argv ) {
  oui_init_options( argc, argv );
  nl_error(0, "Startup");
  load_tmdac(NULL);
  int nQrows = RDR_BUFSIZE/tmi(nbrow);
  if (nQrows < 2) nQrows = 2;
  Reader rdr(nQrows, nQrows/2, RDR_BUFSIZE );
  rdr.control_loop();
  nl_error(0, "Shutdown");
}

Reader::Reader(int nQrows, int low_water, int bufsize) :
    data_generator(nQrows, low_water), data_client( bufsize, 0, NULL ) {
  it_blocked = 0;
  ot_blocked = 0;
  if ( sem_init( &it_sem, 0, 0) || sem_init( &ot_sem, 0, 0 ) )
    nl_error( 3, "Semaphore initialization failed" );
  int rv = pthread_mutex_init( &dq_mutex, NULL );
  if ( rv )
    nl_error( 3, "Mutex initialization failed: %s",
            strerror(errno));
  init_tm_type();
  nl_assert(input_tm_type == TMTYPE_DATA_T3);
  mlf = mlf_init( 3, 60, 0, "./LOG", "dat", NULL );
}

static void pt_create( void (*func)(void *), void *arg ) {
  int rv = pthread_create( NULL, NULL, func, arg );
  if ( rv != EOK )
    nl_error(3,"pthread_create failed: %s", strerror(errno));
}

void Reader::control_loop() {
  pt_create( output_thread, this );
  pt_create( input_thread, this );
  rdr::data_generator.operate();
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

void Reader::service_row_timer() {
  lock();
  if ( ot_blocked == OT_BLOCKED_TIME ) {
    ot_blocked = 0;
    post(&ot_sem);
  }
  unlock();
}

void Reader::event(enum dg_event evt) {
  lock();
  switch (evt) {
    case dg_event_start:
      if (ot_blocked == OT_BLOCKED_STOPPED) {
        ot_blocked = 0;
        post(&ot_sem);
      }
      break;
    case dg_event_stop:
      if (ot_blocked == OT_BLOCKED_TIME || ot_blocked == OT_BLOCKED_DATA) {
        ot_blocked = 0;
        post(&ot_sem);
      }
      break;
    case dg_event_quit:
      dc_quit = true;
      if ( ot_blocked ) {
        ot_blocked = 0;
        post(&ot_sem);
      }
      if ( it_blocked ) {
        it_blocked = 0;
        post(&it_sem);
      }
      break;
    case dg_event_fast:
      if ( ot_blocked == OT_BLOCKED_TIME || ot_blocked == OT_BLOCKED_STOPPED ) {
        ot_blocked = 0;
        post(&ot_sem);
      }
      break;
    default:
      break;
  }
  unlock();
}

void *input_thread(void *Reader_ptr ) {
  Reader *DGr = (Reader *)Reader_ptr;
  return DGr->input_thread();
}

void *Reader::input_thread() {
  while (!dc_quit)
    read();
  return NULL;
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
          if ( it_blocked == IT_BLOCKED_DATA ) {
            it_blocked = 0;
            sem_post(&it_sem);
          }
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
  MFCtr = data->mfctr;
  // Can check here for time limits
  while ( n_rows && !dc_quit ) {
    char *dest;
    lock();
    int n_room = allocate_rows(&dest);
    if ( n_room ) {
      unlock();
      if ( n_room > n_rows ) n_room = n_rows;
      int rawsize = n_room*nbQrow;
      memcpy( dest, raw, rawsize );
      commit_rows( MFCtr, 0, int n_room );
      raw += rawsize;
      n_rows -= n_room;
      MFCtr += n_room;
    } else {
      it_blocked = IT_DATA_BLOCKED;
      unlock();
      sem_wait(&it_sem);
    }
  }
}

// Return non-zero where there is nothing else to read
// This is absolutely a first cut. It will stop at the first sign of trouble (i.e. a missing file)
// What I will want is a record of first file and last file and/or first time/last time
int Reader::process_eof() {
  if ( bfr_fd != -1 ) close(bfr_fd);
  int nlrl = set_response(0);
  bfr_fd = mlf_next_fd( mlf );
  set_response(nlrl);
  if ( bfr_fd == -1 ) {
    dc_quit = true;
    return 1;
  }
  return 0;
}
