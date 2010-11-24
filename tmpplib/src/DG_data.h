#ifndef DG_DATA_H
#define DG_DATA_H

struct data_dev_attr;
#define IOFUNC_ATTR_T struct data_dev_attr
#define IOFUNC_OCB_T struct data_dev_ocb

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

struct data_dev_ocb {
  iofunc_ocb_t hdr;
  int rcvid;
  int msgsize;
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
    struct data_dev_ocb *blocked;
    static resmgr_connect_funcs_t connect_funcs;
    static resmgr_io_funcs_t io_funcs;
    static bool funcs_initialized;
    static bool quitting;
  public:
    // DG_data(data_generator *data_gen); // perhaps I only need the dispatch
    DG_data(DG_dispatch *disp, const char *name, void *data,
        int data_size, int synch);
    ~DG_data();
    int ready_to_quit(); // virtual function of DG_dispatch_client
    int io_write(resmgr_context_t *ctp, IOFUNC_OCB_T *ocb, int nonblock);
    void synch();
    int stale();
};

extern "C" {
  int DG_data_io_write( resmgr_context_t *ctp,
            io_write_t *msg, IOFUNC_OCB_T *ocb );
  int DG_data_io_notify( resmgr_context_t *ctp,
	    io_notify_t *msg, IOFUNC_OCB_T *ocb );
  int DG_data_io_close_ocb( resmgr_context_t *ctp,
	    void *rsvd, IOFUNC_OCB_T *ocb );
}

#endif

