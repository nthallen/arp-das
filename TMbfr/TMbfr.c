#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include "oui.h"
#include "TMbfr.h"

int io_read (resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb);
int iofunc_open_hook( resmgr_context_t *ctp, io_open_t *msg,
                      RESMGR_HANDLE_T *handle, void *extra);

static resmgr_connect_funcs_t    connect_funcs;
static resmgr_io_funcs_t         io_funcs;
static iofunc_attr_t             attr;

static struct ocb *ocb_calloc (resmgr_context_t *ctp, struct device *device) {
  ocb_t ocb = calloc( 1, sizeof(struct ocb) );
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

int
timer_tick(message_context_t *ctp, int code, unsigned flags, void *handle) {
    /* union sigval value = ctp->msg->pulse.value; */
	attr.nbytes += 12;
	run_readq();
    return 0;
}

main(int argc, char **argv) {
    /* declare variables we'll be using */
    resmgr_attr_t        resmgr_attr;
    dispatch_t           *dpp;
    dispatch_context_t   *ctp;
    thread_pool_attr_t   pool_attr;
    thread_pool_t        *tpp;
    int                  id;
	struct sigevent      event;
	struct _itimer       itime;
	int                  timer_id;

	oui_init_options( argc, argv );

    /* initialize dispatch interface */
    if((dpp = dispatch_create()) == NULL) {
        fprintf(stderr, "%s: Unable to allocate dispatch handle.\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    /* initialize resource manager attributes */
    memset(&resmgr_attr, 0, sizeof resmgr_attr);
    resmgr_attr.nparts_max = DRBFR_NPARTS_MAX;
    resmgr_attr.msg_max_size = DRBFR_MSG_MAX;

    /* initialize functions for handling messages */
    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, 
                     _RESMGR_IO_NFUNCS, &io_funcs);
	/* io_funcs.read = io_read; */
	/* io_funcs.write = io_write; */
    connect_funcs.open = iofunc_open_hook;

    /* initialize attribute structure used by the device */
    iofunc_attr_init(&attr, S_IFNAM | 0644, 0, 0);
    attr.nbytes = 0;
	attr.mount = &mountpoint;

    /* Check Experiment variable for sanity: \w[\w.]* */
    /* Build device name */
    /* attach our device name */
    id = resmgr_attach(dpp,            /* dispatch handle        */
                       &resmgr_attr,   /* resource manager attrs */
                       "/dev/sample",  /* device name            */
                       _FTYPE_ANY,     /* open type              */
                       0,              /* flags                  */
                       &connect_funcs, /* connect routines       */
                       &io_funcs,      /* I/O routines           */
                       &attr);         /* handle                 */
    if(id == -1) {
        fprintf(stderr, "%s: Unable to attach name.\n", argv[0]);
        return EXIT_FAILURE;
    }

	/* Initialize an event structure, and attach a pulse to it */
    if((event.sigev_code = pulse_attach(dpp, MSG_FLAG_ALLOC_PULSE, 0, &timer_tick,
                                        NULL)) == -1) {
        fprintf(stderr, "Unable to attach timer pulse.\n");
         return EXIT_FAILURE;
    }

    /* Connect to our channel */
    if((event.sigev_coid = message_connect(dpp, MSG_FLAG_SIDE_CHANNEL)) == -1) {
        fprintf(stderr, "Unable to attach to channel.\n");
        return EXIT_FAILURE;
    }

    event.sigev_notify = SIGEV_PULSE;
    event.sigev_priority = -1;
    /* We could create several timers and use different sigev values for each */
    event.sigev_value.sival_int = 0;

    if((timer_id = TimerCreate(CLOCK_REALTIME, &event)) == -1) {;
        fprintf(stderr, "Unable to attach channel and connection.\n");
        return EXIT_FAILURE;
    }

    /* And now setup our timer to fire every second */
    itime.nsec = 1000000000;
    itime.interval_nsec = 1000000000;
    TimerSettime(timer_id, 0,  &itime, NULL);

    /* initialize thread pool attributes */
    memset(&pool_attr, 0, sizeof pool_attr);
    pool_attr.handle = dpp;
    pool_attr.context_alloc = dispatch_context_alloc;
    pool_attr.block_func = dispatch_block;
    pool_attr.handler_func = dispatch_handler;
    pool_attr.context_free = dispatch_context_free;
    pool_attr.lo_water = 2;
    pool_attr.hi_water = 4;
    pool_attr.increment = 1;
    pool_attr.maximum = 50;     /* allocate a thread pool handle */
    if((tpp = thread_pool_create(&pool_attr,
                                 POOL_FLAG_EXIT_SELF)) == NULL) {
        fprintf(stderr, "%s: Unable to initialize thread pool.\n",
                argv[0]);
        return EXIT_FAILURE;
    }     /* start the threads, will not return */
    thread_pool_start(tpp);
}

int iofunc_open_hook( resmgr_context_t *ctp, io_open_t *msg,
                      RESMGR_HANDLE_T *handle, void *extra) {
  msg->?
  iofunc_open_default( ctp, msg, handle, extra );
}

int
io_read (resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb)
{
    int         nleft;
    int         nbytes;
    int         nparts;
    int         status;
    int         nonblock;

    if ((status = iofunc_read_verify (ctp, msg, ocb, &nonblock)) != EOK)
        return (status);
        
    if (msg->i.xtype & _IO_XTYPE_MASK != _IO_XTYPE_NONE)
        return (ENOSYS);

    enq_read( ctp->rcvid, msg->i.nbytes, ocb, nonblock );
    return( _RESMGR_NOREPLY );

    #ifdef OLD_APPROACH
    /*
     *  on all reads (first and subsequent) calculate
     *  how many bytes we can return to the client,
     *  based upon the number of bytes available (nleft)
     *  and the client's buffer size
     */

    nleft = ocb->attr->nbytes - ocb->offset;
    nbytes = min (msg->i.nbytes, nleft);

    if (nbytes > 0) {
        /* set up the return data IOV */
        SETIOV (ctp->iov, buffer + ocb->offset, nbytes);

        /* set up the number of bytes (returned by client's read()) */
        _IO_SET_READ_NBYTES (ctp, nbytes);

        /*
         * advance the offset by the number of bytes
         * returned to the client.
         */

        ocb->offset += nbytes;
        
        nparts = 1;
    } else {
        /*
         * they've asked for zero bytes or they've already previously
         * read everything
         */
        
        _IO_SET_READ_NBYTES (ctp, 0);
        
        nparts = 0;
    }

    /* mark the access time as invalid (we just accessed it) */

    if (msg->i.nbytes > 0)
        ocb->attr->flags |= IOFUNC_ATTR_ATIME;

    return (_RESMGR_NPARTS (nparts));
    #endif /* OLD_APPROACH */
}
