#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#include "nortlib.h"
#include "company.h"
#include "subbusd_int.h"
#include "nl_assert.h"

static resmgr_connect_funcs_t    connect_funcs;
static resmgr_io_funcs_t         io_funcs;
static iofunc_attr_t             attr;
#define DEVNAME "/dev/" COMPANY "/subbus"

static int saw_int = 0;

static int subbus_io_msg(resmgr_context_t *ctp, io_msg_t *msg,
               RESMGR_OCB_T *ocb) {
  subbusd_msg_t sbdmsg;
  
  MsgRead (ctp->rcvid, &sbdmsg, sizeof (sbdmsg), 0);
  if (sbdmsg.hdr.mgrid != SUBBUSD_MGRID)
    return (ENOSYS);
  incoming_sbreq( ctp->rcvid, sbdmsg.request );
  return (_RESMGR_NOREPLY);
}

void sigint_handler( int sig ) {
  saw_int = 1;
}

int main(int argc, char **argv) {
  resmgr_attr_t        resmgr_attr;
  dispatch_t           *dpp;
  dispatch_context_t   *ctp;
  int                  id;

  /* initialize dispatch interface */
  if((dpp = dispatch_create()) == NULL) {
      nl_error(3,
              "%s: Unable to allocate dispatch handle.\n",
              argv[0]);
      return EXIT_FAILURE;
  }

  /* initialize resource manager attributes */
  memset(&resmgr_attr, 0, sizeof resmgr_attr);
  resmgr_attr.nparts_max = 1;
  resmgr_attr.msg_max_size = 2048;

  /* initialize functions for handling messages */
  iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, 
                   _RESMGR_IO_NFUNCS, &io_funcs);

  io_funcs.msg = subbus_io_msg;

  /* initialize attribute structure used by the device */
  iofunc_attr_init(&attr, S_IFNAM | 0666, 0, 0);

  /* attach our device name */
  id = resmgr_attach(
          dpp,            /* dispatch handle        */
          &resmgr_attr,   /* resource manager attrs */
          DEVNAME,        /* device name            */
          _FTYPE_ANY,     /* open type              */
          0,              /* flags                  */
          &connect_funcs, /* connect routines       */
          &io_funcs,      /* I/O routines           */
          &attr);         /* handle                 */
  if (id == -1)
      nl_error( 3, "%s: Unable to attach name.\n", argv[0]);
  init_subbus(dpp);
  nl_error( 0, "Initialized" );

  signal( SIGINT, sigint_handler );

  /* allocate a context structure */
  ctp = dispatch_context_alloc(dpp);

  /* start the resource manager message loop */
  while(saw_int == 0) {
    if((ctp = dispatch_block(ctp)) == NULL) {
      if (errno == EINTR) break;
      nl_error( 3, "block error: %s", strerror(errno));
      return EXIT_FAILURE;
    }
    dispatch_handler(ctp);
  }
  nl_error( 0, "Shutting down" );
  shutdown_subbus();
  return 0;
}

