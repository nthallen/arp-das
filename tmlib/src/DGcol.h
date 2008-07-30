#ifndef COLLECTOR_H_INCLUDED
#define COLLECTOR_H_INCLUDED
#include "DG_data.h"
#include <list>

/**
  TSCHK_CHECK: set if we need to make some sort of timestamp check
  TSCHK_REQUIRED: set if we have determined that we must issue a new timestamp
  TSCHK_RTIME: The check to be performed is based on a need to resynch with realtime
  TSCHK_IMPLICIT: The timestamp is required due to MFCtr rollover
 */
#define TSCHK_RTIME 1
#define TSCHK_IMPLICIT 2
#define TSCHK_CHECK 4
#define TSCHK_REQUIRED 8

class collector : public data_generator {
  public:
    collector();
    virtual ~collector();
    void init();
    void event(enum dg_event evt);
    void receive(char *name, void *data, int data_size, int synch);
  protected:
    void service_timer();
    void single_step();
    void Collect_Row();
    void commit_tstamp( mfc_t MFCtr, time_t time );
  private:
    int rowlets;
    static unsigned short majf_row, minf_row;
    unsigned short next_minor_frame;
    short ts_checks;
    void ts_check(); // don't know how to get to this one from a timer
    void init_senders();
    std::list<DG_data *> data_clients;
};

#endif


