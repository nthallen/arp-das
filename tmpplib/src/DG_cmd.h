#ifndef DG_CMD_H
#define DG_CMD_H

#include <signal.h>
#include "DG.h"

class data_generator;

class DG_cmd : public DG_dispatch_client {
  private:
    int dev_id;
    data_generator *dg;
    static iofunc_attr_t cmd_attr;
        static resmgr_connect_funcs_t connect_funcs;
        static resmgr_io_funcs_t io_funcs;
  public:
    DG_cmd(data_generator *data_gen);
    ~DG_cmd();
    void attach(); // add to dispatch list
    int execute(char *buf);
    int ready_to_quit(); // virtual function of DG_dispatch_client
    static int const DG_CMD_BUFSIZE = 80;
};

extern "C" {
    int DG_cmd_io_write(
        resmgr_context_t *ctp,
	io_write_t *msg,
	RESMGR_OCB_T *ocb );
}

#endif

