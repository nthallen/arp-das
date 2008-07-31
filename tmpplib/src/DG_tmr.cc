/**
 * DG_tmr Object definitions
 */
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <sys/iomsg.h>
#include "tm.h"
#include "DG_Resmgr.h"
#include "DG_cmd.h"
#include "DG_tmr.h"

int DG_tmr_pulse_func( message_context_t *ctp, int code,
        unsigned flags, void *handle ) {
  DG_tmr *tmr = (DG_tmr *)handle;
  tmr->dg->service_timer();
  return 0;
}

DG_tmr::DG_tmr(data_generator *data_gen) {
  timerid = -1;
  dg = data_gen;
  struct timespec ts;
  if (clock_getres(CLOCK_REALTIME, &ts))
    nl_error(4, "Error from clock_getres()");
  timer_resolution_nsec = timespec2nsec(&ts);
}

void DG_tmr::attach() {
  struct sigevent tmr_ev;
  int rc;

  pulse_code =
    pulse_attach( dg->dispatch->dpp, MSG_FLAG_ALLOC_PULSE, 0, DG_tmr_pulse_func, this );
  if ( pulse_code < 0 )
    nl_error(3, "Error %d from pulse_attach", errno );
  int coid = message_connect( dg->dispatch->dpp, MSG_FLAG_SIDE_CHANNEL );
  if ( coid == -1 )
    nl_error(3, "Error %d from message_connect", errno );
  tmr_ev.sigev_notify = SIGEV_PULSE;
  tmr_ev.sigev_coid = coid;
  tmr_ev.sigev_priority = getprio(0);
  tmr_ev.sigev_code = pulse_code;
  rc = timer_create( CLOCK_REALTIME, &tmr_ev, &timerid );
  if ( rc < 0 ) nl_error( 3, "Error creating timer" );
  DG_dispatch_client::attach(dg->dispatch);
}

int DG_tmr::ready_to_quit() {
  if ( timerid != -1 ) {
    if ( pulse_detach(dg->dispatch->dpp, pulse_code, 0) == -1 ) {
      nl_error( 2, "pulse_detach returned -1" );
    }
    if ( timer_delete(timerid) == -1 ) {
      nl_error( 2, "timer_delete returned errno %d", errno );
    }
    timerid = -1;
  }
  return 1;
}

DG_tmr::~DG_tmr() {
  nl_error( 0, "Destructing DG_tmr object" );
}

void DG_tmr::settime( int per_sec, int per_nsec ) {
  struct itimerspec itime;

  itime.it_value.tv_sec = itime.it_interval.tv_sec = per_sec;
  itime.it_value.tv_nsec = itime.it_interval.tv_nsec = per_nsec;
  timer_settime(timerid, 0, &itime, NULL);
}

void DG_tmr::settime( uint64_t per_nsec ) {
  struct itimerspec itime;
  nsec2timespec( &itime.it_value, per_nsec );
  itime.it_interval = itime.it_value;
  timer_settime(timerid, 0, &itime, NULL);
}
