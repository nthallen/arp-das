#ifndef DG_H_INCLUDED
#define DG_H_INCLUDED

#include "DQ.h"
#include "DG_Resmgr.h"
#include "DG_cmd.h"
#include "DG_tmr.h"

enum dg_event { dg_event_start, dg_event_stop, dg_event_fast, dg_event_quit };

class data_queue;
class DG_cmd;
class DG_tmr;

class data_generator : public data_queue {
  public:
    data_generator(int nQrows, int low_water);
    virtual ~data_generator();
    void init( int collection );
    void operate(); // event loop
    int execute(char *cmd);
    virtual void event(enum dg_event evt);
    DG_dispatch *dispatch;
    virtual void service_row_timer() = 0;
  protected:
    bool quit; // non-zero means we are terminating
    bool started; // True while running
    bool regulated; // True whenever data flow is time-based
    bool autostart;
    bool regulation_optional;

    // virtual void single_step() = 0;
    void transmit_data( int single_row );
    int dg_bfr_fd;
    DG_cmd *cmd;
    DG_tmr *tmr;
  private:
    void tm_start(int lock_needed = 1);
    void tm_play(int lock_needed = 1);
    void tm_stop();
    uint64_t row_period_nsec_default;
    uint64_t row_period_nsec_current;
    void check_writev( int rc, int wr_size, char *where );
};

#endif
