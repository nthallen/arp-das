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
#include "oui.h"
#include "tm.h"

static resmgr_connect_funcs_t    connect_funcs;
static resmgr_io_funcs_t         io_funcs;
static iofunc_attr_t             attr;
#define DEVNAME_DEFAULT "/dev/" COMPANY "/subbus"
char const *subbusd_devname = DEVNAME_DEFAULT;

static int SB_Shutdown = 0;

static int subbus_write(resmgr_context_t *ctp, io_write_t *msg,
               RESMGR_OCB_T *ocb) {
  SB_Shutdown = 1;
  return iofunc_write_default(ctp, msg, ocb);
}

static int subbus_io_msg(resmgr_context_t *ctp, io_msg_t *msg,
               RESMGR_OCB_T *ocb) {
  subbusd_req_t sbdmsg;
  int nb, nb_exp;
  
  nb = MsgRead(ctp->rcvid, &sbdmsg, sizeof(sbdmsg), 0);
  if ( nb < sizeof(subbusd_req_hdr_t) ||
       sbdmsg.sbhdr.iohdr.mgrid != SUBBUSD_MGRID ||
       sbdmsg.sbhdr.sb_kw != SB_KW)
    return ENOSYS;
  /* check the size of the incoming message */
  switch ( sbdmsg.sbhdr.command ) {
    case SBC_WRITEACK:
    case SBC_WRITECACHE:
      nb_exp = sizeof(subbusd_req_data0); break;
    case SBC_SETCMDENBL:
    case SBC_SETCMDSTRB:
    case SBC_SETFAIL:
    case SBC_READACK:
    case SBC_READCACHE:
      nb_exp = sizeof(subbusd_req_data1); break;
    case SBC_READSW:
    case SBC_READFAIL:
    case SBC_GETCAPS:
    case SBC_TICK:
    case SBC_DISARM:
      nb_exp = 0; break;
    case SBC_QUIT:
      SB_Shutdown = 1;
      nb_exp = 0; break;
    case SBC_INTATT:
      nb_exp = sizeof(subbusd_req_data2); break;
    case SBC_INTDET:
      nb_exp = sizeof(subbusd_req_data3); break;
    case SBC_MREAD:
      nb_exp = 3*sizeof(unsigned short);
      if ( nb >= nb_exp ) {
        nl_assert( sbdmsg.data.d4.req_len >= nb_exp );
        nb_exp = sbdmsg.data.d4.req_len;
      }
      break;
    default:
      return ENOSYS;
  }
  nb_exp += sizeof(subbusd_req_hdr_t);
  if ( nb < nb_exp )
    nl_error( 4, "Received short message for command %d: Expected %d received %d",
      sbdmsg.sbhdr.command, nb_exp, nb );
  incoming_sbreq( ctp->rcvid, &sbdmsg );
  return (_RESMGR_NOREPLY);
}

void sigint_handler( int sig ) {
  SB_Shutdown = 1;
}

int main(int argc, char **argv) {
  resmgr_attr_t        resmgr_attr;
  dispatch_t           *dpp;
  dispatch_context_t   *ctp;
  int                  id;

  oui_init_options(argc, argv);
  sb_cache_init();

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
  io_funcs.write = subbus_write; // For Quit only

  /* initialize attribute structure used by the device */
  iofunc_attr_init(&attr, S_IFNAM | 0666, 0, 0);

  /* attach our device name */
  id = resmgr_attach(
          dpp,             /* dispatch handle        */
          &resmgr_attr,    /* resource manager attrs */
          tm_dev_name(subbusd_devname), /* device name            */
          _FTYPE_ANY,      /* open type              */
          0,               /* flags                  */
          &connect_funcs,  /* connect routines       */
          &io_funcs,       /* I/O routines           */
          &attr);          /* handle                 */
  if (id == -1)
      nl_error( 3, "%s: Unable to attach name.\n", argv[0]);
  init_subbus(dpp);
  nl_error( 0, "Initialized" );

  signal( SIGINT, sigint_handler );
  signal( SIGHUP, sigint_handler );

  /* allocate a context structure */
  ctp = dispatch_context_alloc(dpp);

  /* start the resource manager message loop */
  while (1) {
    if((ctp = dispatch_block(ctp)) == NULL) {
      if (errno == EINTR) break;
      nl_error( 3, "block error: %s", strerror(errno));
      return EXIT_FAILURE;
    }
    dispatch_handler(ctp);
    if (SB_Shutdown == 1 && ctp->resmgr_context.rcvid == 0
	  && attr.count == 0)
      break;
  }
  nl_error( 0, "Shutting down" );
  shutdown_subbus();
  return 0;
}

