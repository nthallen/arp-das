/* dccc_if.c defines how we get commands */

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "disc_cmd.h"
#include "nortlib.h"
#include "tm.h"

#ifndef DCCC_RESMGR

  static void receive_cmd( int cmd_fd, cmd_t *pcmd ) {
    char tbuf[DCCC_MAX_CMD_BUF+1];

    int nb = read( cmd_fd, tbuf, DCCC_MAX_CMD_BUF );
    if ( nb == -1 )
      nl_error( 3, "Error reading from command interface: %s",
        strerror(errno));
    parse_cmd(tbuf, nb, pcmd);
  }

  void operate(void) {
    int cmd_fd;
    cmd_fd = tm_open_name( tm_dev_name("cmd/dccc"), NULL, O_RDONLY );
    while (!DCCC_Done) {
      cmd_t pcmd;
      receive_cmd(cmd_fd, &pcmd );
    }
  } 

#else /* DCCC_RESMGR */

  #include <stdio.h>
  #include <stddef.h>
  #include <stdlib.h>
  #include <sys/iofunc.h>
  #include <sys/dispatch.h>

  static int io_write(resmgr_context_t *ctp, io_write_t *msg,
                      RESMGR_OCB_T *ocb);

  static resmgr_connect_funcs_t    connect_funcs;
  static resmgr_io_funcs_t         io_funcs;
  static iofunc_attr_t             attr;

  /* our ocb allocating & freeing functions */
  // static iofunc_funcs_t ocb_funcs = {
  //  _IOFUNC_NFUNCS,
  //  ocb_calloc,
  //  ocb_free
  //};

  /* the mount structure, we have only one so we statically declare it */
  // iofunc_mount_t mountpoint = { 0, 0, 0, 0, &ocb_funcs };

  void operate(void) {
    /* declare variables we'll be using */
    resmgr_attr_t        resmgr_attr;
    dispatch_t           *dpp;
    dispatch_context_t   *ctp;
    int                  id;

    /* initialize dispatch interface */
    if ((dpp = dispatch_create()) == NULL)
      nl_error(3, "Unable to allocate dispatch handle." );

    /* initialize resource manager attributes */
    memset(&resmgr_attr, 0, sizeof resmgr_attr);
    resmgr_attr.nparts_max = 1;
    resmgr_attr.msg_max_size = DCCC_MAX_CMD_BUF;

    /* initialize functions for handling messages */
    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, 
                     _RESMGR_IO_NFUNCS, &io_funcs);
    /* io_funcs.read = io_read; */
    io_funcs.write = io_write;
    // connect_funcs.open = iofunc_open_hook;

    /* initialize attribute structure used by the device */
    iofunc_attr_init(&attr, S_IFNAM | 0664, 0, 0);
    attr.nbytes = 0;
    // attr.mount = &mountpoint;

    /* Check Experiment variable for sanity: \w[\w.]* */
    /* Build device name */
    /* attach our device name */
    id = resmgr_attach(dpp,            /* dispatch handle        */
                       &resmgr_attr,   /* resource manager attrs */
                       tm_dev_name("dccc"),  /* device name            */
                       _FTYPE_ANY,     /* open type              */
                       0,              /* flags                  */
                       &connect_funcs, /* connect routines       */
                       &io_funcs,      /* I/O routines           */
                       &attr);         /* handle                 */
    if (id == -1)
      nl_error(3, "Unable to attach name.");

    ctp = dispatch_context_alloc(dpp);
    if ( ctp == NULL )
      nl_error(3, "dispatch_context_alloc() failed" );

    seteuid(getuid()); // become unprivileged

    while ( 1 ) {
      if ((ctp = dispatch_block(ctp)) == NULL) {
        nl_error(4, "block_error" );
      }
      dispatch_handler(ctp);
      if ( DCCC_Done && ctp->resmgr_context.rcvid == 0 &&
            attr.count == 0 ) break;
    }
  }

  int io_write( resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb ) {
    int status, msgsize;
    char buf[DCCC_MAX_CMD_BUF+1];
    cmd_t pcmd;

    status = iofunc_write_verify(ctp, msg, (iofunc_ocb_t *)ocb, NULL);
    if ( status != EOK )
      return status;

    if ((msg->i.xtype &_IO_XTYPE_MASK) != _IO_XTYPE_NONE )
      return ENOSYS;

    _IO_SET_WRITE_NBYTES( ctp, msg->i.nbytes );

    /* My strategy for the moment will be to only write the first MEMO_BUF_SIZE
       characters. Later, I will loop somehow */
    msgsize = msg->i.nbytes;
    if ( msgsize > DCCC_MAX_CMD_BUF ) {
      msgsize = DCCC_MAX_CMD_BUF;
      nl_error(2, "Received message size %d: Truncated to %d", msg->i.nbytes, msgsize);
    }
    resmgr_msgread( ctp, buf, msgsize, sizeof(msg->i) );
    parse_cmd( buf, msgsize, &pcmd );
    // Could return an error code here, but the log messages are
    // probably sufficient

    if ( msg->i.nbytes > 0)
      ocb->attr->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;
    return _RESMGR_NPARTS(0);
  }
#endif
