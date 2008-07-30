#ifndef DG_TMR_H
#define DG_TMR_H

#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <time.h>
#include <stdint.h>
#include "DG_Resmgr.h"
#include "nortlib.h"
#include "DG.h"

class data_generator;

class DG_tmr : public DG_dispatch_client {
  public:
    DG_tmr(data_generator *data_gen);
    ~DG_tmr();
    void attach();
    void settime( int per_sec, int per_nsec );
    void settime( uint64_t nsecs );
    int ready_to_quit();
    data_generator *dg;
    uint64_t timer_resolution_nsec;
  private:
    int timerid; // set to -1 before initialization and after cleanup
    int pulse_code;
};

extern "C" {
    int DG_tmr_pulse_func( message_context_t *ctp, int code,
        unsigned flags, void *handle );
}

#endif

