#ifndef DG_DATA_H
#define DG_DATA_H

struct data_dev_attr;
#define IOFUNC_ATTR_T struct data_dev_attr

#include <signal.h>
#include "DG.h"

class data_generator;
class DG_data;

struct data_dev_attr {
  iofunc_attr_t attr;
  iofunc_notify_t notify[3];
  bool written;
  DG_data *DGd;
};


class DG_data : public DG_dispatch_client {
  private:
    int dev_id;
    const char *name; // Keep around mostly for debugging
    short int stale_count;
    void *dptr;
    int dsize;
    bool synched;
    struct data_dev_attr data_attr;
    static resmgr_connect_funcs_t connect_funcs;
    static resmgr_io_funcs_t io_funcs;
    static bool funcs_initialized;
  public:
    // DG_data(data_generator *data_gen); // perhaps I only need the dispatch
    DG_data(DG_dispatch *disp, const char *name, void *data,
        int data_size, int synch);
    ~DG_data();
    int ready_to_quit(); // virtual function of DG_dispatch_client
    int io_write(resmgr_context_t *ctp);
    void synch();
    int stale();
};

extern "C" {
  int DG_data_io_write( resmgr_context_t *ctp,
            io_write_t *msg, RESMGR_OCB_T *ocb );
  int DG_data_io_notify( resmgr_context_t *ctp,
	    io_notify_t *msg, RESMGR_OCB_T *ocb );
  int DG_data_io_close_ocb( resmgr_context_t *ctp,
	    void *rsvd, RESMGR_OCB_T *ocb );
}

#endif

