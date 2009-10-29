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
#include <limits.h>
#include <stdlib.h>
#include "nortlib.h"
#include "nl_assert.h"
#include "tm.h"

resmgr_connect_funcs_t DG_data::connect_funcs;
resmgr_io_funcs_t DG_data::io_funcs;
bool DG_data::funcs_initialized = false;

extern "C" {
  static struct data_dev_ocb *ocb_calloc(resmgr_context_t *ctp,
		data_dev_attr *device);
  static void ocb_free(struct data_dev_ocb *ocb);
}

static iofunc_funcs_t ocb_funcs = {
  _IOFUNC_NFUNCS,
  ocb_calloc,
  ocb_free
};

static iofunc_mount_t DGdata_mountpoint = { 0, 0, 0, 0, &ocb_funcs };

static struct data_dev_ocb *ocb_calloc(resmgr_context_t *ctp,
	      data_dev_attr *device) {
  struct data_dev_ocb *ocb =
    (struct data_dev_ocb *)
      malloc(sizeof(struct data_dev_ocb));
  ocb->rcvid = 0;
  ocb->msgsize = 0;
  return ocb;
}

static void ocb_free(struct data_dev_ocb *ocb) {
  free(ocb);
}




DG_data::DG_data(DG_dispatch *dispatch, const char *name_in, void *data,
	 int data_size, int synch)
    : DG_dispatch_client() {

  dispatch_t *dpp = dispatch->dpp;
  name = name_in;
  dptr = data;
  dsize = data_size;
  synched = synch;
  stale_count = 0;
  data_attr.written = false;
 
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
    io_funcs.notify = DG_data_io_notify;
    io_funcs.close_ocb = DG_data_io_close_ocb;
    //io_funcs.unblock = DG_data_io_unblock;
    funcs_initialized = true;
  }
  
  iofunc_attr_init( &data_attr.attr, S_IFNAM | 0222, 0, 0 ); // write-only
  data_attr.attr.mount = &DGdata_mountpoint;
  IOFUNC_NOTIFY_INIT( data_attr.notify );
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

// The semantics of write are a bit non-standard. I use blocking I/O
// purely as a synchronization technique. In all contexts, a write
// that gets here is immediately processed, but depending on
// the synchronization flag (set in the TMC source) and the nonblock
// value (set by the client), we may not reply immediately to the
// client.
int DG_data::io_write( resmgr_context_t *ctp, IOFUNC_OCB_T *ocb,
			  int nonblock ) {
  int msgsize = resmgr_msgread( ctp, dptr, dsize, sizeof(io_write_t) );
  data_attr.written = synched;
  if (synched && !nonblock) {
    if (blocked && blocked != ocb)
      return EBUSY;
    ocb->rcvid = ctp->rcvid;
    ocb->msgsize = msgsize;
    blocked = ocb;
    return _RESMGR_NOREPLY;
  }
  _IO_SET_WRITE_NBYTES( ctp, msgsize );
  stale_count = 0;
  return EOK;
}

void DG_data::synch() {
  data_attr.written = false;
  if (blocked) {
    MsgReply(blocked->rcvid, blocked->msgsize, 0, 0);
    blocked = 0;
  }
  if (IOFUNC_NOTIFY_OUTPUT_CHECK( data_attr.notify, 1 ) )
    iofunc_notify_trigger(data_attr.notify, 1, IOFUNC_NOTIFY_OUTPUT);
}

int DG_data::stale() {
  int rv = stale_count;
  if ( stale_count < SHRT_MAX ) ++stale_count;
  return rv;
}

int DG_data_io_write( resmgr_context_t *ctp,
         io_write_t *msg, IOFUNC_OCB_T *ocb ) {
  int status, nonblock;

  status = iofunc_write_verify(ctp, msg, (iofunc_ocb_t *)ocb, &nonblock);
  if ( status != EOK )
    return status;

  if ((msg->i.xtype &_IO_XTYPE_MASK) != _IO_XTYPE_NONE )
    return ENOSYS;

  return ocb->hdr.attr->DGd->io_write(ctp, ocb, nonblock);
}

int DG_data_io_notify( resmgr_context_t *ctp,
         io_notify_t *msg, IOFUNC_OCB_T *ocb ) {
  IOFUNC_ATTR_T *wr_attr = ocb->hdr.attr;
  int trig = 0;
  if (!wr_attr->written) trig |= _NOTIFY_COND_OUTPUT;
  return(iofunc_notify(ctp, msg, wr_attr->notify, trig, NULL, NULL ));
}

int DG_data_io_close_ocb(resmgr_context_t *ctp, void *rsvd,
			  IOFUNC_OCB_T *ocb) {
  IOFUNC_ATTR_T *wr_attr = ocb->hdr.attr;
  iofunc_notify_remove(ctp, wr_attr->notify);
  return(iofunc_close_ocb_default(ctp, rsvd, &ocb->hdr));
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
  if ( data_attr.attr.count )
    nl_error( -2, "Still waiting for DG/data/%s", name );
  return data_attr.attr.count == 0;
}
