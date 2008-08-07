#ifndef DGCOL_H_INCLUDED
#define DGCOL_H_INCLUDED
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

/**
 * collector is the portion of the collection processing that
 * is implemented in the tmpplib.
 */
class collector : public data_generator {
  public:
    collector();
    virtual ~collector();
    void init();
    void event(enum dg_event evt);
    void receive(char *name, void *data, int data_size, int synch);
  protected:
    void commit_tstamp( mfc_t MFCtr, time_t time );
    static unsigned short majf_row, minf_row;
    unsigned short next_minor_frame;
  private:
    std::list<DG_data *> data_clients;
};

/**
 * tmc_collector is the portion of the collection processing that
 * is implemented in source code generated by tmc (colmain.skel)
 */
class tmc_collector : public collector {
  public:
    void service_row_timer();
    void init_senders();
    void event(enum dg_event evt);
  private:
    void ts_check(); // don't know how to get to this one from a timer
    short ts_checks;
    int rowlets;
};

#endif

