#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include "DG_Resmgr.h"
#include "DG_cmd.h"
#include "nortlib.h"
#include "nl_assert.h"
#include "tm.h"

static DG_cmd *Cmd;
resmgr_connect_funcs_t DG_cmd::connect_funcs;
resmgr_io_funcs_t DG_cmd::io_funcs;
iofunc_attr_t DG_cmd::cmd_attr;

/**
 * buf is guaranteed to be nul-terminated
 * We will strip any trailing newlines before forwarding to dg->execute()
 */
int DG_cmd::execute(char *buf) {
  assert(buf != 0);
  int len = strlen(buf);
  while ( len > 0 && isspace(buf[len-1]) )
    buf[--len] = '\0';
  return dg->execute(buf);
}

DG_cmd::DG_cmd(data_generator *data_gen) : DG_dispatch_client() {
  dg = data_gen;
}

void DG_cmd::attach() {
  dispatch_t *dpp = dg->dispatch->dpp;
  if (Cmd != NULL)
      nl_error(3,"Only one DG_cmd instance allowed");
 
  // This is our write-only command interface
  resmgr_attr_t resmgr_attr;
  memset(&resmgr_attr, 0, sizeof(resmgr_attr));
  resmgr_attr.nparts_max = 1;
  resmgr_attr.msg_max_size = DG_CMD_BUFSIZE+1;

  iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs,
      _RESMGR_IO_NFUNCS, &io_funcs );
  io_funcs.write = DG_cmd_io_write;
  
  iofunc_attr_init( &DG_cmd::cmd_attr, S_IFNAM | 0222, 0, 0 ); // write-only
  char *wr_devname = tm_dev_name( "DG/cmd" );
  dev_id = resmgr_attach( dpp, &resmgr_attr, wr_devname, _FTYPE_ANY, 0,
                    &DG_cmd::connect_funcs, &DG_cmd::io_funcs, &DG_cmd::cmd_attr );
  if ( dev_id == -1 )
    nl_error( 3, "Unable to attach name %s: errno %d", wr_devname, errno );
 
  Cmd = this;
  DG_dispatch_client::attach(dg->dispatch); // Now get in on the quit loop
}

DG_cmd::~DG_cmd() { }

int DG_cmd_io_write( resmgr_context_t *ctp,
         io_write_t *msg, RESMGR_OCB_T *ocb ) {
  int status, msgsize;
  char buf[DG_cmd::DG_CMD_BUFSIZE+1];

  status = iofunc_write_verify(ctp, msg, (iofunc_ocb_t *)ocb, NULL);
  if ( status != EOK )
    return status;

  if ((msg->i.xtype &_IO_XTYPE_MASK) != _IO_XTYPE_NONE )
    return ENOSYS;

  msgsize = msg->i.nbytes;
  if ( msgsize > DG_cmd::DG_CMD_BUFSIZE )
    return E2BIG;

  _IO_SET_WRITE_NBYTES( ctp, msg->i.nbytes );

  resmgr_msgread( ctp, buf, msgsize, sizeof(msg->i) );
  buf[msgsize] = '\0';

  // Handle the message
  Cmd->execute(buf);
  return EOK;
}

/** DG_cmd::ready_to_quit() returns true if we are ready to terminate. For DG/cmd, that means all writers
  have closed their connections and we have detached the device.
*/
int DG_cmd::ready_to_quit() {
  // unlink the name
  if ( dev_id != -1 ) {
    int rc = resmgr_detach( dispatch->dpp, dev_id, _RESMGR_DETACH_PATHNAME );
    if ( rc == -1 )
      nl_error( 2, "Error returned from resmgr_detach: %d", errno );
    dev_id = -1;
  }
  // ### Need to make sure my data_generator knows it's time to quit
  return cmd_attr.count == 0;
}
