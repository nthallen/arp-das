#include "Collector.h"

collector::collector() : data_generator(4,1) {
  regulated = true;
  regulation_optional = false;
}

collector::~collector() {}

void collector::init() {
  data_generator::init( 1 );
  init_senders();
  tminitfunc();
}

void collector::service_timer() {
  Collect_Row();
  transmit_data(0);
}

void collector::single_step() {
  service_timer();
}

void collector::event(enum dg_event evt) {
  if ( evt == dg_event_start ) {
    rowlets = 0;
    next_minor_frame = majf_row = 0;
    minf_row = 0;
    ts_checks =  TSCHK_RTIME | TSCHK_REQUIRED;
  }
}

void collector::commit_tstamp( mfc_t MFCtr, time_t time ) {
  tm_info.t_stmp.mfc_num = MFCtr;
  tm_info.t_stmp.secs = time;
  data_generator::commit_tstamp(MFCtr, time);
}

/**
 * Collect_Row() is reponsible for:
 * -determining whether a new timestamp is required
 * -filling in/defining the minor fram counter and synch
 * -populating the row of data
 * New timestamp may be required because:
 * -we just started
 * -the minor frame counter is rolling over
 * -we are greater than TS_MFC_LIMIT minor frames from the old timestamp
 * -we have drifted from realtime somehow
 * Implemented in colmain.skel
 */

void collector::receive(char *name, void *data, int data_size, int synch) {
  DG_data *DGd = new DG_data(dispatch, name, data, data_size, synch);
  data_clients.push_back(DGd);
}

