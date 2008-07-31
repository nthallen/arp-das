// Include DG_data.h first here to make sure our definition of
// IOFUNC_ATTR_T gets used in this file
#include "DG_data.h"
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include "nortlib.h"
#include "nl_assert.h"
#include "tm.h"

resmgr_connect_funcs_t DG_data::connect_funcs;
resmgr_io_funcs_t DG_data::io_funcs;
bool DG_data::funcs_initialized = false;

DG_data::DG_data(DG_dispatch *dispatch, char *name_in, void *data,
	 int data_size, int synch)
    : DG_dispatch_client() {

  dispatch_t *dpp = dispatch->dpp;
  name = name_in;
  dptr = data;
  dsize = data_size;
  synched = synch;
 
  // This is our write-only command interface
  resmgr_attr_t resmgr_attr;
  memset(&resmgr_attr, 0, sizeof(resmgr_attr));
  resmgr_attr.nparts_max = 1;
  resmgr_attr.msg_max_size = data_size;
  data_attr.DGd = this;

  // io_funcs and connect_funcs are static, so we only need ot do this once,
  // but it won't hurt to do it more than once
  if ( !funcs_initialized ) {
    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs,
        _RESMGR_IO_NFUNCS, &io_funcs );
    io_funcs.write = DG_data_io_write;
    //io_funcs.notify = DG_data_io_notify;
    //io_funcs.unblock = DG_data_io_unblock;
    funcs_initialized = true;
  }
  
  iofunc_attr_init( &data_attr.attr, S_IFNAM | 0222, 0, 0 ); // write-only
  char tbuf[80];
  snprintf(tbuf, 79, "DG/data/%s", name);
  char *wr_devname = tm_dev_name( tbuf );
  dev_id = resmgr_attach( dpp, &resmgr_attr, wr_devname, _FTYPE_ANY, 0,
                    &connect_funcs, &io_funcs, &data_attr );
  if ( dev_id == -1 )
    nl_error( 3, "Unable to attach name %s: errno %d", wr_devname, errno );
  
  attach(dispatch); // Now get in on the quit loop
}

DG_data::~DG_data() {}

int DG_data::io_write( resmgr_context_t *ctp ) {
  int msgsize = resmgr_msgread( ctp, dptr, dsize, sizeof(io_write_t) );
  _IO_SET_WRITE_NBYTES( ctp, msgsize );
  written = true;
  return EOK;
}

int DG_data_io_write( resmgr_context_t *ctp,
         io_write_t *msg, RESMGR_OCB_T *ocb ) {
  int status;

  status = iofunc_write_verify(ctp, msg, (iofunc_ocb_t *)ocb, NULL);
  if ( status != EOK )
    return status;

  if ((msg->i.xtype &_IO_XTYPE_MASK) != _IO_XTYPE_NONE )
    return ENOSYS;

  return ocb->attr->DGd->io_write(ctp);
}

/** DG_data::ready_to_quit() returns true if we are ready to terminate. For DG/data, that means all writers
  have closed their connections and we have detached the device.
*/
int DG_data::ready_to_quit() {
  // unlink the name
  if ( dev_id != -1 ) {
    int rc = resmgr_detach( dispatch->dpp, dev_id, _RESMGR_DETACH_PATHNAME );
    if ( rc == -1 )
      nl_error( 2, "Error returned from resmgr_detach: %d", errno );
    dev_id = -1;
  }
  return data_attr.attr.count == 0;
}
