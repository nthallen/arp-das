#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>

int io_read (resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb);


static resmgr_connect_funcs_t    connect_funcs;
static resmgr_io_funcs_t         io_funcs;
static iofunc_attr_t             attr;

#define MAX_REQS 10
struct {
  int rcvid;
  int32_t req_bytes;
  RESMGR_OCB_T *ocb;
  int nonblock;
} readq[MAX_REQS];

void run_readq( void ) {
  int i;
  for ( i = 0; i < MAX_REQS; i++ ) {
    if ( readq[i].ocb != 0 ) {
      RESMGR_OCB_T *ocb = readq[i].ocb;
      int32_t left = ocb->attr->nbytes - ocb->offset;
      int nbytes = min( readq[i].req_bytes, left );
      if ( nbytes > 0 ) {
        char buf[20];
        long int mfctr = ( ocb->offset/12 ) % 100000l;
        int off = ocb->offset % 12;
        int nb = 12 - off;
        sprintf( buf, "MFCtr %5ld\n", mfctr );
        ocb->offset += nb;
		ocb->attr->flags |= IOFUNC_ATTR_ATIME;
        MsgReply( readq[i].rcvid, nb, &buf[off], nb );
        readq[i].ocb = NULL;
      } else if ( readq[i].nonblock || readq[i].req_bytes == 0 ) {
        MsgReply( readq[i].rcvid, readq[i].nonblock ? EAGAIN : 0, 0, 0 );
        readq[i].ocb = NULL;
      }
    }
  }
}

void enq_read( int rcvid, int32_t req_bytes, RESMGR_OCB_T *ocb, int nonblock ) {
  int i;
  /* add request to the list and then run the list */
  for ( i = 0; i < MAX_REQS; i++ ) {
    if ( readq[i].ocb == 0 ) {
      readq[i].rcvid = rcvid;
      readq[i].req_bytes = req_bytes;
      readq[i].nonblock = nonblock;
      readq[i].ocb = ocb;
      run_readq();
      return;
    }
  }
  MsgReply( rcvid, EAGAIN, 0, 0 );
}

int
timer_tick(message_context_t *ctp, int code, unsigned flags, void *handle) {
    /* union sigval value = ctp->msg->pulse.value; */
	attr.nbytes += 12;
	run_readq();
    return 0;
}

main(int argc, char **argv)
{
    /* declare variables we'll be using */
    resmgr_attr_t        resmgr_attr;
    dispatch_t           *dpp;
    dispatch_context_t   *ctp;
    int                  id;
	struct sigevent      event;
	struct _itimer       itime;
	int                  timer_id;

    /* initialize dispatch interface */
    if((dpp = dispatch_create()) == NULL) {
        fprintf(stderr, "%s: Unable to allocate dispatch handle.\n",
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
	io_funcs.read = io_read;

    /* initialize attribute structure used by the device */
    iofunc_attr_init(&attr, S_IFNAM | 0444, 0, 0);
    attr.nbytes = 0;

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

    /* allocate a context structure */
    ctp = dispatch_context_alloc(dpp);

    /* start the resource manager message loop */
    while(1) {
        if((ctp = dispatch_block(ctp)) == NULL) {
            fprintf(stderr, "block error\n");
            return EXIT_FAILURE;
        }
        dispatch_handler(ctp);
    }
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
