#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "oui.h"
#include "memo.h"
#include "tm.h"
#include "nortlib.h"

//int io_read (resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb);
int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb);

static resmgr_connect_funcs_t    connect_funcs;
static resmgr_io_funcs_t         io_funcs;
static iofunc_attr_t             attr;
static FILE *ofp, *ofp2;
static char *output_filename;
static int opt_V = 0;

static struct ocb *ocb_calloc (resmgr_context_t *ctp, IOFUNC_ATTR_T *device) {
  ocb_t *ocb = calloc( 1, sizeof(ocb_t) );
  if ( ocb == 0 ) return 0;
  /* Initialize any other elements. Currently all zeros is good. */
  return ocb;
}

static void ocb_free (struct ocb *ocb) {
  /* Be sure to remove this from the blocking list:
     Actually, there really is no way it should be on
     the blocking list. */
  if ( ocb->hold_index )
    nl_error( 2, "hold_index non-zero in ocb_free" );
  free( ocb );
}

static iofunc_funcs_t ocb_funcs = { /* our ocb allocating & freeing functions */
    _IOFUNC_NFUNCS,
    ocb_calloc,
    ocb_free
};

/* the mount structure, we have only one so we statically declare it */
iofunc_mount_t mountpoint = { 0, 0, 0, 0, &ocb_funcs };

void memo_init_options( int argc, char **argv ) {
  int c;
  ofp = stdout;
  ofp2 = NULL;
  optind = OPTIND_RESET;
  opterr = 0;
  while ((c = getopt(argc, argv, opt_string)) != -1 ) {
    switch (c) {
      case 'o':
        output_filename = optarg;
        break;
      case 'V':
	opt_V = 1;
	break;
      case '?':
	nl_error( 3, "Unrecognized commandline option -%c", optopt );
	break;
      default:
        break;
    }
  }
}

int main(int argc, char **argv) {
    /* declare variables we'll be using */
    resmgr_attr_t        resmgr_attr;
    dispatch_t           *dpp;
    int                  id;

    oui_init_options( argc, argv );

    /* initialize dispatch interface */
    if((dpp = dispatch_create()) == NULL) {
        nl_error(3, "%s: Unable to allocate dispatch handle.\n",
                argv[0]);
    }

    /* initialize resource manager attributes */
    memset(&resmgr_attr, 0, sizeof resmgr_attr);
    resmgr_attr.nparts_max = DRBFR_NPARTS_MAX;
    resmgr_attr.msg_max_size = DRBFR_MSG_MAX;

    /* initialize functions for handling messages */
    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, 
                     _RESMGR_IO_NFUNCS, &io_funcs);
    /* io_funcs.read = io_read; */
    io_funcs.write = io_write;
    // connect_funcs.open = iofunc_open_hook;

    /* initialize attribute structure used by the device */
    iofunc_attr_init(&attr, S_IFNAM | 0664, 0, 0);
    attr.nbytes = 0;
    attr.mount = &mountpoint;

    /* Check Experiment variable for sanity: \w[\w.]* */
    /* Build device name */
    /* attach our device name */
    id = resmgr_attach(dpp,            /* dispatch handle        */
                       &resmgr_attr,   /* resource manager attrs */
                       tm_dev_name("memo"),  /* device name            */
                       _FTYPE_ANY,     /* open type              */
                       0,              /* flags                  */
                       &connect_funcs, /* connect routines       */
                       &io_funcs,      /* I/O routines           */
                       &attr);         /* handle                 */
    if(id == -1) {
        nl_error(3, "%s: Unable to attach name.\n", argv[0]);
    }

                // running starts at 2. When the first client opens a connection,
          // we change to 1. When all connections are closed, we go to 0.
          // In this way, we don't require any additional commanding, but
          // we do require that there be some continuity in how the clients
          // use the resource.
    int running = 2;
    dispatch_context_t   *ctp;
    ctp = dispatch_context_alloc(dpp);

    seteuid(getuid()); // become unprivileged
    if ( output_filename ) {
      ofp = fopen( output_filename, "a" );
      if ( ofp == NULL )
        nl_error( 3, "Unable to open output file '%s'",
           output_filename );
      if (opt_V) ofp2 = stdout;
    }
    { time_t now = time(NULL);
      const char *nowt = asctime(gmtime(&now));
      fprintf( ofp, "\nMemo Starting: %s", nowt );
      if (ofp2)
	fprintf( ofp2, "\nMemo Starting: %s", nowt );
    }
    signal(SIGHUP, SIG_IGN);
    while ( running ) {
      if ((ctp = dispatch_block(ctp)) == NULL) {
        fprintf( ofp, "Memo internal: block error\n" );
        if (ofp2)
	  fprintf( ofp2, "Memo internal: block error\n" );
        return EXIT_FAILURE;
      }
      dispatch_handler(ctp);
      if ( running > 1 && attr.count > 0 ) running = 1;
      else if ( running == 1 && attr.count == 0 ) running = 0;
    }
    fprintf( ofp, "Memo Terminating\n" );
    fclose(ofp);
    if (ofp2) {
      fprintf( ofp2, "Memo Terminating\n" );
      fclose(ofp2);
    }
    return 0;
}

#define MEMO_BUF_SIZE 256

int io_write( resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb ) {
  int status, msgsize;
  char buf[MEMO_BUF_SIZE+1];

  status = iofunc_write_verify(ctp, msg, (iofunc_ocb_t *)ocb, NULL);
  if ( status != EOK )
    return status;

  if ((msg->i.xtype &_IO_XTYPE_MASK) != _IO_XTYPE_NONE )
    return ENOSYS;

  _IO_SET_WRITE_NBYTES( ctp, msg->i.nbytes );

  /* My strategy for the moment will be to only write the first MEMO_BUF_SIZE
     characters. Later, I will loop somehow */
  msgsize = msg->i.nbytes;
  if ( msgsize > MEMO_BUF_SIZE ) msgsize = MEMO_BUF_SIZE;
  resmgr_msgread( ctp, buf, msgsize, sizeof(msg->i) );
  buf[msgsize] = '\0';
  if ( msgsize > 0 && buf[msgsize-1] == '\n' )
    buf[msgsize-1] = '\0';
  // parse message to identify:
  //  timestamp severity appmnc message
  //  create timestamp if missing
  //  output result
  fprintf(ofp, "%s\n", buf );
  fflush(ofp);
  if ( ofp2 ) {
    fprintf(ofp2, "%s\n", buf );
    fflush(ofp2);
  }

  if ( msg->i.nbytes > 0)
    ocb->hdr.attr->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;
  return _RESMGR_NPARTS(0);
}
