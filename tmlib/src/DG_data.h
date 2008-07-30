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
  DG_data *DGd;
};


class DG_data : public DG_dispatch_client {
  private:
    int dev_id;
    char *name; // Keep around mostly for debugging
    void *dptr;
    int dsize;
    bool written;
    bool synched;
    struct data_dev_attr data_attr;
    static resmgr_connect_funcs_t connect_funcs;
    static resmgr_io_funcs_t io_funcs;
    static bool funcs_initialized;
  public:
    // DG_data(data_generator *data_gen); // perhaps I only need the dispatch
    DG_data(DG_dispatch *disp, char *name, void *data, int data_size, int synch);
    ~DG_data();
    int ready_to_quit(); // virtual function of DG_dispatch_client
    int io_write(resmgr_context_t *ctp);
};

extern "C" {
  int DG_data_io_write( resmgr_context_t *ctp,
            io_write_t *msg, RESMGR_OCB_T *ocb );
}

#endif

