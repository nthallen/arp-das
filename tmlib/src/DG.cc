/* DG.c */
#include <errno.h>
#include <sys/uio.h>
#include "DG.h"

data_generator::data_generator(int nQrows, int low_water)
    : data_queue(nQrows,low_water) {
  bfr_fd = -1;
  quit = false;
  started = false;
  regulated = false;
  regulation_optional = true;
  autostart = false;
}

data_generator::~data_generator() {}

/**
 * Assumes tm_info is defined
 */
void data_generator::init(int collection) {
  data_queue::init();
  bfr_fd = open(tm_dev_name("TM/DG"), collection ? O_WRONLY|O_NONBLOCK : O_WRONLY );
  // bfr_fd = open("TM_DG.log", O_WRONLY|O_CREAT );
  if (bfr_fd < 0) nl_error(3, "Unable to open TM/DG: %d", errno );
  tm_hdr_t hdr = { TMHDR_WORD, TMTYPE_INIT };
  iov_t iov[2];
  SETIOV(&iov[0], &hdr, sizeof(hdr));
  SETIOV(&iov[1], &tm_info, sizeof(tm_info));
  int rc = writev( bfr_fd, iov, 2);
  check_writev( rc, sizeof(tm_info)+sizeof(hdr), "sending TMTYPE_INIT" );
  dispatch = new DG_dispatch();
  cmd = new DG_cmd(this);
  cmd->attach();
  tmr = new DG_tmr(this);
  tmr->attach();
  row_period_nsec_default = tmi(nsecsper)*(uint64_t)1000000000L/tmi(nrowsper);
  row_period_nsec_current = row_period_nsec_default;
}

void data_generator::transmit_data( int single_row ) {
  // We can read from the queue without locking
  dq_tstamp_ref *dqts;
  dq_data_ref *dqdr;
  // nl_error( 0, "transmit_data(%s)", single_row ? "single" : "all" );
  int rc;
  tm_hdrs_t hdrs;
  hdrs.s.hdr.tm_id = TMHDR_WORD;
  iov_t iov[3];
  while ( first_dqr ) {
    switch ( first_dqr->type ) {
      case dq_tstamp:
        dqts = (dq_tstamp_ref *)first_dqr;
        hdrs.s.hdr.tm_type = TMTYPE_TSTAMP;
        SETIOV(&iov[0], &hdrs, sizeof(tm_hdr_t));
        SETIOV(&iov[1], &dqts->TS, sizeof(dqts->TS));
        rc = writev(bfr_fd, iov, 2);
        check_writev( rc, sizeof(tm_hdr_t)+sizeof(dqts->TS),
	   "transmitting tstamp" );
        retire_tstamp(dqts);
        break;
      case dq_data:
        dqdr = (dq_data_ref *)first_dqr;
        if (dqdr->n_rows == 0) {
          if (dqdr->next_dqr) retire_rows(dqdr, 0);
          else return;
        } else {
          hdrs.s.hdr.tm_type = output_tm_type;
          int n_rows = single_row ? 1 : dqdr->n_rows;
          hdrs.s.u.dhdr.n_rows = n_rows;
          hdrs.s.u.dhdr.mfctr = dqdr->MFCtr_start;
          hdrs.s.u.dhdr.rownum = dqdr->row_start;
          SETIOV(&iov[0], &hdrs, nbDataHdr);
          int n_iov;
          if ( dqdr->Qrow + n_rows < total_Qrows ) {
            SETIOV(&iov[1], row[dqdr->Qrow], n_rows * nbQrow );
            n_iov = 2;
          } else {
            int n_rows1 = total_Qrows - dqdr->Qrow;
            SETIOV(&iov[1], row[dqdr->Qrow], n_rows1 * nbQrow );
            int n_rows2 = n_rows - n_rows1;
            SETIOV(&iov[2], row[0], n_rows2 * nbQrow );
            n_iov = 3;
          }
          rc = writev(bfr_fd, iov, n_iov);
          check_writev( rc, nbDataHdr + n_rows * nbQrow,
	     "trasmitting data" );
          retire_rows(dqdr, n_rows);
          if ( single_row ) return;
        }
        break;
      default:
        nl_error(4, "Invalid type in transmit_data" );
    }
  }
}

void data_generator::check_writev( int rc, int wr_size, char *where ) {
  if ( rc < 0 ) nl_error( 3, "Error %d %s", errno, where );
  else if ( rc != wr_size )
    nl_error( 3, "writev %d, not %d, %s", rc, wr_size, where );
}

/**
 * Control initialization
 * This is how 
 */
void data_generator::operate() {
  if ( autostart ) tm_start(1);
  dispatch->Loop();
}

/**
 Interperet data generator commands: I need to abstract the "signal handlers" operation.
 It appears that the key notifications are:
  Start
  Stop
  Quit
  Change to unregulated output (fast forward)
 
 TM start: "TMc"
    Set start condition
    If regulated, program timer
    else stop timer
    if ext_stop, signal handlers
TM end/stop "TMe"
    Set stop condition
    stop timer
    if ext_time, signal handlers
Quit: ""
    Set stop and quit conditions
    stop timer
    if ext_stop or ext_time, signal handlers
TM play "TM>"
    set regulated
    set row_rate to default
    if stopped, do start
    else reprogram timer
TM fast forward "TM}"
    if regulation_optional
      stop timer
      set unregulated
      if stopped, do start
      else if ext_time, signal handlers
TM faster/slower "TM+" "TM-"
    if regulation_optional
      if stopped
        do play
      else if regulated
        increase/decrease row_rate
        program timer
TM single step "TMs"

Command Summary:
  "" Quit
  "TMc" TM Start
  "TMe" TM End/Stop/Pause
  "TMs" Single Step
  "TM>" Play
  "TM+" Faster
  "TM-" Slower
  "TM}" Fast Forward
  
  Still need to add the search functions:
 TM Advance to MFCtr
 TM Advance to Time
 */
int data_generator::execute(char *cmd) {
  if (cmd[0] == '\0') {
    lock();
    started = false;
    quit = true;
    unlock();
    tmr->settime(0);
    event(dg_event_quit);
    return 1;
  }
  if ( cmd[0] == 'T' && cmd[1] == 'M' ) {
    switch ( cmd[2] ) {
      case 'c': tm_start(1); break;
      case 'e': tm_stop(); break;
      case 's':
        if (started) tm_stop();
        else single_step();
        break;
      case '>': tm_play(); break; // play
      case '+':
        if (regulation_optional) {
          lock();
          if (!started) tm_play(0);
          else {
            if ( regulated ) {
              row_period_nsec_current = row_period_nsec_current * 2 / 3;
              if ( row_period_nsec_current < tmr->timer_resolution_nsec ) {
                regulated = false;
                tmr->settime(0);
                unlock();
                event(dg_event_fast);
              } else {
                tmr->settime(row_period_nsec_current);
                unlock();
              }
            } else unlock();
          }
        }
        break;
      case '-': // slower
        if (regulation_optional) {
          lock();
          if (!started) tm_play(0);
          else {
            row_period_nsec_current = row_period_nsec_current * 3 / 2;
            tmr->settime(row_period_nsec_current);
            regulated = true;
            unlock();
          }
        }
        break;
      case '}': // fast forward
        if (regulation_optional) {
          lock();
          tmr->settime(0);
          regulated = false;
          if (started) {
            unlock();
            event(dg_event_fast);
          } else tm_start(0);
        }
        break;
      default:
        nl_error(2,"Invalid TM command in data_generator::execute: '%s'", cmd );
        break;
    }
  } else nl_error(2, "Invalid command in data_generator::execute: '%s'", cmd );
  return 0;
}

void data_generator::event(enum dg_event evt) {}

void data_generator::tm_start(int lock_needed) {
  if (lock_needed) lock();
  started = true;
  if ( regulated ) tmr->settime(row_period_nsec_current);
  else tmr->settime( 0 );
  unlock();
  event( dg_event_start );
}

void data_generator::tm_play(int lock_needed) {
  if (lock_needed) lock();
  regulated = true;
  row_period_nsec_current = row_period_nsec_default;
  if ( started ) {
    tmr->settime(row_period_nsec_current);
    unlock();
  } else tm_start(0); // don't need to re-lock(), but will unlock()
}

void data_generator::tm_stop() {
  lock();
  started = false;
  unlock();
  tmr->settime(0);
  event(dg_event_stop);
}
