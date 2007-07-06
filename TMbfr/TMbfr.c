/* TMbfr2.c */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include "TMbfr.h"
#include "nortlib.h"
#include "nl_assert.h"

static int io_read (resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb);
static int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb);
static void process_tm_info( IOFUNC_OCB_T *ocb );
static int (*data_state_eval)( IOFUNC_OCB_T *ocb, int nonblock );

static resmgr_connect_funcs_t    connect_funcs;
static resmgr_io_funcs_t         rd_io_funcs, wr_io_funcs;
static IOFUNC_ATTR_T             dg_attr, dcf_attr, dco_attr;
static int                       dg_id, dcf_id, dco_id;
static char *                    dg_name, dcf_name, dco_name;
static resmgr_attr_t             resmgr_attr;
static dispatch_t                *dpp;
static pthread_mutex_t           dg_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t           dq_mutex = PTHREAD_MUTEX_INITIALIZER;

data_queue_t Data_Queue;
// TS_queue_t *TS_Queue;
DQD_Queue_t DQD_Queue;
static tm_ocb_t *blocked_writer;

static struct {
  tm_ocb_t *first, *last;
} blocked_readers;

/* enqueue_read() queues the specified ocb.
   Assumes dq is locked.
   If nonblock is set, returns error to the client.
   Should prioritize by some clever scheme including priority, but
   for starters, I'll just FIFO.
 */
static void enqueue_read( IOFUNC_OCB_T *ocb, int nonblock ) {
  if ( nonblock ) {
    if ( MsgError( ocb->rw.read.rcvid, EAGAIN ) == -1 )
      nl_error( 2, "Error %d on MsgError", errno );
  } else {
    if ( blocked_readers.first == NULL ) {
      blocked_readers.first = blocked_readers.last = ocb;
      ocb->next = 0;
    } else {
      assert( blocked_readers.last != NULL );
      blocked_readers.last->next = ocb;
      blocked_readers.last = ocb;
      ocb->next = 0;
    }
  }
}

/* run_read_queue() invokes read_reply() on all the blocked ocbs
     in the read queue.
   assumes dq is unlocked (since read_reply() will lock it as
     needed)
 */
static void run_read_queue(void) {
  tm_ocb_t *rq;
  lock_dq();
  rq = blocked_readers.first;
  blocked_readers.first = blocked_readers.last = NULL;
  unlock_dq();
  while ( rq ) {
    tm_ocb_t *cur_rq = rq;
    rq = rq->next;
    cur_rq->next = NULL;
    read_reply(ocb, 0);
  }
}

/* dq_deref() reduces reference count by one, does any work
   associated with ref count dropping to zero and returns the
   next dq_descriptor. dq must be locked.
 */
static dq_descriptor *dq_deref( dq_descriptor *dqd ) {
  dq_descriptor *next_dqd = dqd->next;
  if ( --dqd->ref_count <= 0 && next_dqd != NULL
       && dqd->n_Qrows == 0 && DQD_Queue.first == dqd ) {
    /* Can expire this dqd */
    if ( --dqd->TSq->ref_count <= 0 ) {
      // Tsq is no longer referenced
      free_memory(TSq);
      free_memory(dqd);
      DQD_Queue.first = next_dqd;
    }
  }
  return next_dqd;
}

static void lock_dq(void) {
  int rv = pthread_mutex_lock( &dq_mutex );
  if ( rv != EOK )
    nl_error( 4, "Error %d locking dq mutex", rv );
}

static void unlock_dq(void) {
  int rv = pthread_mutex_unlock( &dq_mutex );
  if ( rv != EOK )
    nl_error( 4, "Error %d unlocking dq mutex", rv );
}

static tm_ocb_t *ocb_calloc(resmgr_context_t *ctp, IOFUNC_ATTR_T *device) {
  tm_ocb_t *ocb = calloc( 1, sizeof(tm_ocb_t) );
  if ( ocb == 0 ) return 0;
  /* Initialize any other elements. */
  ocb->next_ocb = 0;
  // ocb->rw.read.buf = 0;
  ocb->data.dqd = 0;
  ocb->data.n_Qrows = 0;
  ocb->rw.read.rows_missing = 0;
  ocb->state = TM_STATE_HDR; // ### do this in a state init function?
  ocb->part.nbdata = sizeof(ocb->part.hdr); // ### and this
  ocb->part.dptr = (char *)&ocb->part.hdr; // ### and this
  return ocb;
}

static void ocb_free(struct ocb *ocb) {
  /* Be sure to remove this from the blocking list:
     Actually, there really is no way it should be on
     the blocking list. */
  assert( ocb->rw.read.rcvid == 0 );
  assert( ocb->next_ocb == 0 );
  lock_dq();
  dq_deref(ocb->data.dqd);
  unlock_dq();
  if ( ocb.rw.read.buf ) free(ocb.rw.read.buf);
  free( ocb );
}

static iofunc_funcs_t ocb_funcs = { /* our ocb allocating & freeing functions */
    _IOFUNC_NFUNCS,
    ocb_calloc,
    ocb_free
};

/* the mount structure, we have only one so we statically declare it */
static iofunc_mount_t mountpoint = { 0, 0, 0, 0, &ocb_funcs };

static int setup_mount( char *namebase, int node_type, int mode,
	IOFUNC_ATTR_T *attr,  resmgr_iofuncs_t *funcs ) {
  char *server_name;
  int mnt_id;

  iofunc_attr_init((iofunc_attr_t *)attr, S_IFNAM | mode, 0, 0);
  attr->attr.nbytes = 0;
  attr->attr.mount = &mountpoint;
  attr->node_type = node_type;

  server_name = tm_dev_name( namebase );
  mnt_id = resmgr_attach(dpp,            /* dispatch handle        */
		     &resmgr_attr,   /* resource manager attrs */
		     server_name,    /* device name            */
		     _FTYPE_ANY,     /* open type              */
		     0,              /* flags                  */
		     &connect_funcs, /* connect routines       */
		     &funcs,         /* I/O routines           */
		     attr);          /* handle                 */
  if( mnt_id == -1 )
    nl_error( 3, "Unable to attach name '%s'", server_name );
  return mnt_id;
}

static void shutdown_mount( int id, char *name ) {
  int rv = resmgr_detach( dpp, id, _RESMGR_DETACH_ALL );
  if ( rv < 0 )
    nl_error( 2, "Error %d from resmgr_detach(%s)", rv, name );
}

static int dg_opened = 0;

// all_closed() returns non-zero if all OCBs are closed
static int all_closed(void) {
  return dg_attr.attr.count == 0 &&
         dcf_attr.attr.count == 0 &&
	 cdo_attr.attr.count == 0;
}

int main(int argc, char **argv ) {
  int use_threads = 0;
  char *server_name;

  /* initialize dispatch interface */
  if((dpp = dispatch_create()) == NULL) {
      nl_error(3, "Unable to allocate dispatch handle.");
  }

  /* initialize resource manager attributes. */
  /* planning to share this struct between rd and wr */
  memset(&resmgr_attr, 0, sizeof resmgr_attr);
  // resmgr_attr.nparts_max = 0;
  // resmgr_attr.msg_max_size = 0;

  /* initialize functions for handling messages */
  iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, 
		   _RESMGR_IO_NFUNCS, &rd_io_funcs);
  rd_io_funcs.read = io_read;
  /* Will want to handle _IO_NOTIFY at least */
  // rd_io_funcs.notify = io_notify;

  iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, 
		   _RESMGR_IO_NFUNCS, &wr_io_funcs);
  wr_io_funcs.write = io_write;
  connect_funcs.open = io_open;

  dg_id = setup_mount( "TM/DG", TM_DG, 0664, &dg_attr, &wr_io_funcs );
  dcf_id = setup_mount( "TM/DCf", TM_DCf, 0444, &dcf_attr, &rd_io_funcs );
  dco_id = setup_mount( "TM/DCo", TM_DCo, 0444, &dco_attr, &rd_io_funcs );

  if ( use_threads ) {
    thread_pool_attr_t   pool_attr;
    thread_pool_t        *tpp;

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
	nl_error(3, "Unable to initialize thread pool");
    }     /* start the threads, will not return */
    thread_pool_start(tpp);
  } else {
    dispatch_context_t   *ctp;
    ctp = dispatch_context_alloc(dpp);
    while ( 1 ) {
      if ((ctp = dispatch_block(ctp)) == NULL) {
	nl_error( 2, "block error\n" );
	return;
      }
      // printf( "  type = %d,%d  attr.count = %d\n",
      //   ctp->resmgr_context.msg->type,
      //   ctp->resmgr_context.msg->connect.subtype, attr.count );
      dispatch_handler(ctp);
      if ( dg_opened && ctp->resmgr_context.rcvid == 0
	    && all_closed() ) {
	break;
      }
    }
  }
  shutdown_mount( dg_id, "TM/DG" );
  shutdown_mount( dcf_id, "TM/DCf" );
  shutdown_mount( dco_id, "TM/DCo" );
  return;
}

static int io_open( resmgr_context_t *ctp, io_open_t *msg,
                    IOFUNC_ATTR_T *attr, void *extra ) {
  // Check to make sure it isn't already open
  if ( attr->node_type == TM_DG ) {
    int rv = pthread_mutex_lock( &dg_mutex );
    if ( rv == EOK ) {
      int count = attr->attr.count;
      if ( count == 0 && dg_opened == 0 ) {
	rv = iofunc_open_default( ctp, msg, &attr->attr, extra );
	if ( rv == EOK ) {
	  assert( attr->attr.count );
	  dg_opened = 1;
	}
      } else rv = EBUSY;
      pthread_mutex_unlock( &dg_mutex );
      return rv;
    } else nl_error( 3, "pthread_mutex_lock returned error %d", rv );
  }
  return iofunc_open_default( ctp, msg, &attr->attr, extra );
}

// This is where data is recieved.
static int io_write( resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb ) {
  int status, nonblock;
  char buf[CMD_MAX_COMMAND_IN+1];

  status = iofunc_write_verify(ctp, msg, (iofunc_ocb_t *)ocb, &nonblock);
  if ( status != EOK )
    return status;

  if ((msg->i.xtype &_IO_XTYPE_MASK) != _IO_XTYPE_NONE )
    return ENOSYS;

  // iofunc_write_verify should have stopped us if this is a DC node,
  // since they have no write permissions, but just check to make sure.
  assert( ocb->hdr.attr->node_type == TM_DG );

  // Assert this isn't a combine message
  if (msg->i.combine_len & _IO_COMBINE_FLAG)
    return (ENOSYS);
  assert(ctp->offset == 0);

  // Store the information we need from the message
  ocb->rw.write.nb_msg = msg->i.nbytes;
  ocb->rw.write.off_msg = 0; // sizeof(msg->i);
  ocb->rw.write.rcvid = ctp->rcvid;

  do_write( ocb, nonblock );
  return _RESMGR_NOREPLY;
}

static void do_write( IOFUNC_OCB_T *ocb, int nonblock ) {
  int new_rows = 0;
  
  // _IO_SET_WRITE_NBYTES( ctp, msg->i.nbytes );
  // Not necessary since we'll handle the return ourselves
  // However, that means we need to store the total message size
  // Use off_msg for total size.
  
  while ( ocb->rw.write.nb_msg > 0 && ocb->part.nbdata > 0 ) {
    int nbread;
    nbread = ocb->rw.write.nb_msg < ocb->part.nbdata ?
      ocb->rw.write.nb_msg : ocb->part.nbdata;
    if ( MsgRead( ocb->rw.write.rcvid, ocb->part.dptr, nbread,
          ocb->rw.write.off_msg + sizeof(msg->i) ) < 0 ) {
      // These errors are all pretty serious, and since writes
      // come from our DG, they are very bad news.
      // Retrying MsgRead() doesn't make sense, and if we ignore
      // this message, we won't know where we are when the next
      // message comes in. A reasonable assumption would be that
      // the next message would start at the beginning of a header,
      // but that's not guaranteed.
      nl_error( 2, "Error %d from MsgRead: %s", errno, strerror(errno) );
      MsgError( ocb->rw.write.rcvid, EFAULT );
      ocb->state = TM_STATE_HDR;
      ocb->part.nbdata = sizeof( ocb->part.hdr );
      ocb->part.dptr = (char *)&ocb->part.hdr;
      return;
    }
    ocb->part.dptr += nbread;
    ocb->part.nbdata -= nbread;
    ocb->rw.write.nb_msg -= nbread;
    ocb->rw.write.off_msg += nbread;
    if ( ocb->state == TM_STATE_DATA ) {
      ocb->rw.write.nb_rec -= nbread;
      ocb->rw.write.off_rec += nbread;
      ocb->rw.write.nb_queue -= nbread;
      ocb->rw.write.off_queue += nbread;
    }
    assert(ocb->part.nbdata >= 0);
    if ( ocb->part.nbdata == 0 ) {
      switch ( ocb->state ) {
	case TM_STATE_HDR:
	  if ( ocb->part.hdr.s.hdr.tm_id != TMHDR_WORD )
	    return EINVAL;
	  switch ( ocb->part.hdr.s.hdr.tm_type ) {
	    case TMTYPE_INIT:
	      //### eliminate use of part.off here
	      ocb->part.off = sizeof(tm_hdrs_t)-sizeof(tm_hdr_t);
	      ocb->part.nbdata = sizeof(tm_info) - ocb->part.off;
	      ocb->state = TM_STATE_INFO;
	      ocb->part.dptr = (char *)&tm_info;
	      memcpy( ocb->part.dptr, &ocb->part.hdr.raw[sizeof(tm_hdr_t)],
		      ocb->part.off );
	      ocb->part.dptr += ocb->part.off;
	      break;
	    case TMTYPE_TSTAMP:
	      lock_dq();
	      queue_tstamp( &ocb->part.hdr.s.u.TS );
	      unlock_dq();
	      new_rows++;
	      // ocb->state = TM_STATE_HDR; // already there!
	      ocb->part.nbdata = sizeof( ocb->part.hdr );
	      ocb->part.dptr = (char *)&ocb->part.hdr;
	      break;
	    case TMTYPE_DATA_T1:
	    case TMTYPE_DATA_T2:
	    case TMTYPE_DATA_T3:
	    case TMTYPE_DATA_T4:
	      if ( data_state_eval == 0 )
	        data_state_init(ocb);
	      ocb->rw.write.nbrec = ocb->part.hdr.s.u.dhdr.n_rows *
		ocb->rw.write.nbrow_rec;
	      ocb->rw.write.off_rec = 0;
	      ocb->rw.write.nb_queue = 0; // setup in data_state_eval();
	      ocb->rw.write.off_queue = 0;
	      new_rows += data_state_eval(ocb, nonblock);
	      ocb->state = TM_STATE_DATA;
	      break;
	    default:
	      nl_error( 4, "Invalid state" );
	  }
	  break;
	case TM_STATE_INFO:
	  process_tm_info(ocb);
	  new_rows++;
	  ocb->state = TM_STATE_HDR; //### Use state-init function?
	  ocb->part.nbdata = sizeof(ocb->part.hdr);
	  ocb->part.dptr = (char *)&ocb->part.hdr;
	  break;
	case TM_STATE_DATA:
	  new_rows += data_state_eval(ocb, nonblock);
	  if ( ocb->rw.write.nb_rec <= 0 ) {
	    ocb->state = TM_STATE_HDR;
	    ocb->part.nbdata = sizeof(ocb->part.hdr);
	    ocb->part.dptr = (char *)&ocb->part.hdr;
	  }
	  break;
      }
    }
  }
  if ( ocb->rw.write.nb_msg == 0 ) {
    MsgReply( ocb->rw.write.rcvid, ocb->rw.write.off_msg, 0, 0 );
    //### Mark us as not blocked: maybe that's nbdata != 0?
  } else {
    // We must have nbdata == 0 meaning we're going to block
    assert(!nonblock);
  }
  if ( new_rows ) run_read_queue();
}

// data_state_init() determines nbrow_rec, nbhdr_rec,
// nbrec, off_rec, nb_queue, off_queue
// Fixup for extra data bytes read with the header
// is handled in do_write()
static void data_state_init( IOFUNC_OCB_T *ocb ) {
  ocb->rw.write.buf = NULL;
  switch ( ocb->part.hdr.s.hdr.tm_type ) {
    case TMTYPE_DATA_T1:
      ocb->rw.write.nbrow_rec = tmi(nbrow);
      ocb->rw.write.nbhdr_rec = 6; //### Could be mnemonic
      switch ( Data_Queue.output_tm_type ) {
        case TMTYPE_DATA_T1:
	  data_state_eval = data_state_T1T1;
	  break;
	case TMTYPE_DATA_T2:
	  data_state_eval = data_state_T1T2;
	  break;
	case TMTYPE_DATA_T3:
	  ocb->rw.write.buf = new_memory(tmi(nbminf));
	  data_state_eval = data_state_T1T3;
	  break;
	default:
	  nl_error(4, "Invalid output type in data_state_init_once" );
      }
      break;
    case TMTYPE_DATA_T2:
      ocb->rw.write.nbrow_rec = tmi(nbrow);
      ocb->rw.write.nbhdr_rec = 10; //### Could be mnemonic
      data_state_eval = data_state_T2T2;
      break;
    case TMTYPE_DATA_T3:
      ocb->rw.write.nbrow_rec = tmi(nbrow) - 4;
      ocb->rw.write.nbhdr_rec = 8; //### Could be mnemonic
      data_state_eval = data_state_T3T3;
      break;
    case TMTYPE_DATA_T4:
    default:
      nl_error( 4, "Invalid TMTYPE for data: %d",
	ocb->part.hdr.s.hdr.tm_type );
  }
}

// The job of data_state_eval is to decide how big the next
// chunk of data should be and where it should go.
// This involves finding space in the data queue and
// setting part.dptr and part.nbdata. We may need to expire
// rows from the data queue, but if we aren't nonblocking, we
// might block to allow the readers to handle what we've already
// got.
// data_state_eval() does not transfer any data via ReadMessage,
// but it will copy any bytes from the hdr if off_rec == 0.
// Returns the number of rows that have been completed in the
// data queue.

// First check the data we've already moved into the queue
// The rows we read in will always begin at Data_Queue.last,
// which should be consistent with the end of the current
// dqd data set. The number of rows is also guaranteed to
// fit within the Data_Queue without wrapping, so it should
// be safe to add nrrecd to Data_Queue.last, though it may
// be necessary to set last to zero afterwards.
//   T1->T1 just add to current dqd without checks
//   T1->T2 T2 output does require that frames be consecutive, so we
// need to read in the rows and then check them to make sure they belong
// with the previous records.
//   T1->T3 Data will be copied into the partial buffer one row at a
// time, then MFCtr and SYNCH will be extracted, and the rest of the row
// will be copied into the queue. nb_queue and off_queue will need to be
// fudged before and corrected after each row.
//   T2->T2 Copy straight in, then verify continuity with previous
// records.
//   T3->T3 Copy straight in. Verify consecutive, etc.

static int data_state_T1T1( IOFUNC_OCB_T *ocb, int nonblock ) {
  int nrrecd = ocb->rw.write.off_queue/ocb->rw.write.nbrow_rec;
  assert(ocb->part.nbdata == 0);
  if ( nrrecd ) {
    lock_dq();
    Data_Queue.last += nrrecd;
    assert(Data_Queue.last <= Data_Queue.total_Qrows );
    if ( Data_Queue.last == Data_Queue.total_Qrows )
      Data_Queue.last = 0;
    DQD_Queue.last->n_Qrows += nrrecd;
  }
  // Now look at what we have yet to move.
  if ( ocb->rw.write.nb_rec ) {
    int nminf = ocb->rw.write.nb_rec/tmi(nbminf);
    if ( nminf == 0 ) nminf++;
    if ( Data_Queue.last <= Data_Queue.first ) {
      nrowsfree = Data_Queue.first - Data_Queue.last;
    } else nrowsfree = Data_Queue.total_Qrows - Data_Queue.last;
    if there is any room, use it now, otherwise try to retire rows
    If nonblock, retire as much as we need. Otherwise only retire
    truly expired rows.
    // Is there any room?
    // Do we need to retire rows?
    // Do we need to move hdr data into Data_Queue?
  } else {
    ocb->part.nbdata = 0;
  }
}

static int data_state_T1T2( IOFUNC_OCB_T *ocb, int nonblock ) {
  int nrrecd = ocb->rw.write.off_queue/ocb->rw.write.nbrow_rec;
  assert(ocb->part.nbdata == 0);
  if ( nrrecd ) {
    lock_dq();
}

static int data_state_T1T3( IOFUNC_OCB_T *ocb, int nonblock ) {
  int nrrecd = ocb->rw.write.off_queue/ocb->rw.write.nbrow_rec;
  assert(ocb->part.nbdata == 0);
  if ( nrrecd ) {
    lock_dq();
}

static int data_state_T2T2( IOFUNC_OCB_T *ocb, int nonblock ) {
  int nrrecd = ocb->rw.write.off_queue/ocb->rw.write.nbrow_rec;
  assert(ocb->part.nbdata == 0);
  if ( nrrecd ) {
    lock_dq();
}

static int data_state_T3T3( IOFUNC_OCB_T *ocb, int nonblock ) {
  int nrrecd = ocb->rw.write.off_queue/ocb->rw.write.nbrow_rec;
  assert(ocb->part.nbdata == 0);
  if ( nrrecd ) {
    lock_dq();
}

static int data_state_eval( IOFUNC_OCB_T *ocb, int nonblock ) {
  int nrrecd = ocb->rw.write.off_queue/ocb->rw.write.nbrow_rec;
  assert(ocb->part.nbdata == 0);
  if ( nrrecd ) {
    lock_dq();
    switch ( ocb->part.hdr.s.hdr.tm_type ) {
      case TMTYPE_DATA_T1:
	switch ( Data_Queue.output_tm_type ) {
	  case TMTYPE_DATA_T1:
	    Data_Queue.last += nrrecd;
	    assert(Data_Queue.last <= Data_Queue.total_Qrows );
	    if ( Data_Queue.last == Data_Queue.total_Qrows )
	      Data_Queue.last = 0;
	    DQD_Queue.last->n_Qrows += nrrecd;
	    break;
	  case TMTYPE_DATA_T2:
	    assert(nrrecd%tmi(nrowminf) == 0);
	    dqd = DQD_Queue.last;
	    assert(dqd->Row_num == 0)
	    ###
	    I can add nrrecd to Data_Queue, but I need to check
	    MFCtr on each mf to make sure it's consecutive with
	    the current DQD.
	    break;
	  case TMTYPE_DATA_T3:
	    ###
	    break;
	  default:
	    nl_error(4,"Invalid output_tm_type in data_state_eval" );
	}
	break;
      case TMTYPE_DATA_T2:
	###
	break;
      case TMTYPE_DATA_T3:
	###
	break;
      default:
	nl_error(4, "Invalid output_tm_type in data_state_eval" );
    }
    unlock_dq();
  }
  ###...

  // Now look at what we have yet to move
  // Do we need to retire rows?
  // Do we need to move hdr data into Data_Queue?
  
  return nrrecd;
}

/* queue_tstamp(): Create a new tstamp record in the Tstamp_Queue
   and a new dq_descriptor that references it.
   This currently assumes the dq_mutex is locked.
*/
static void queue_tstamp( tstamp_t *ts ) {
  TS_queue_t *new_TS = new_memory(sizeof(TS_queue_t));
  dq_descriptor_t *new_dqd = new_memory(sizeof(dq_descriptor_t));
  // TS_queue_t **old_TS;
  
  new_TS->TS = *ts;
  // new_TS->next = 0;
  new_TS->ref_count = 1;
  // old_TS = &TS_Queue;
  // while ( *old_TS ) old_TS = &(*old_TS)->next;
  // *old_TS = new_TS;

  // ### Probably need a separate function
  new_dqd->next = 0;
  new_dqd->ref_count = 0;
  new_dqd->starting_Qrow = Data_Queue.last;
  new_dqd->n_Qrows = 0;
  new_dqd->Qrows_expired = 0
  new_dqd->TSq = new_TS;
  new_dqd->MFCtr = 0;
  new_dqd->Row_num = 0;
  if ( DQD_Queue.last )
    DQD_Queue.last->next = new_dqd;
  else DQD_Queue.first = new_dqd;
  DQD_Queue.last = new_dqd;
}

/* As soon as tm_info has been received, we can decide what
   data format to output, how much buffer space to allocate
   and in what configuration. We can then create the first
   timestamp record (with the TS in the tm_info) and the
   first dq_descriptor, albeit with no Qrows, but refrencing
   the the first timestamp. Then we can check to see if any
   readers are waiting, and initialize them.
*/
static int process_tm_info( IOFUNC_OBT_T *ocb ) {
  unsigned char *rowptr;
  int i;

  // Perform sanity checks
  if (tmi(nbminf) == 0 ||
      tmi(nbrow) == 0 ||
      tmi(nrowmajf) == 0 ||
      tmi(nrowsper) == 0 ||
      tmi(nsecsper) == 0 ||
      tmi(mfc_lsb) == tmi(mfc_msb) ||
      tmi(mfc_lsb) >= tmi(nbrow) ||
      tmi(mfc_msb) >= tmi(nbrow) ||
      tmi(nbminf) < tmi(nbrow) ||
      tmi(nbminf) % tmi(nbrow) != 0) {
    nl_error( 2, "Sanity Checks failed on incoming stream" );
    return EINVAL;
  }

  // What data format should we output?
  lock_dq();
  Data_Queue.nbQrow = tmi(nbrow);
  if ( tmi(mfc_lsb) == 0 && tmi(mfc_msb) == 1
       && tmi(nrowminf) == 1 ) {
    Data_Queue.output_tm_type = TMTYPE_DATA_T3;
    Data_Queue.nbQrow -= 4;
    Data_Queue.nbDataHdr = 8;
  } else if ( tmi(nrowminf) == 1 ) {
    Data_Queue.output_tm_type = TMTYPE_DATA_T1;
    Data_Queue.nbDataHdr = 6;
  } else {
    Data_Queue.output_tm_type = TMTYPE_DATA_T2;
    Data_Queue.nbDataHdr = 10;
  }
  Data_Queue.pbuf_size = Data_Queue.nbDataHdr + Data_Queue.nbQrow;
  if ( Data_Queue.pbuf_size < sizeof(tm_hdr_t) + sizeof(tm_info_t) )
    Data_Queue.pbuf_size = sizeof(tm_hdr_t) + sizeof(tm_info_t);

  // how much buffer space to allocate?
  // Let's default to one minute's worth, but make sure we get
  // an integral number of minor frames
  Data_Queue.total_Qrows = tmi(nrowminf) *
    ( ( tmi(nrowsper) * 60 + tmi(nsecsper)*tmi(nrowminf) - 1 )
        / (tmi(nsecsper)*tmi(nrowminf)) );
  Data_Queue.total_size =
    Data_Queue.nbQrow * Data_Queue.total_Qrows;
  Data_Queue.first = Data_Queue.last = 0;
  Data_Queue.raw = new_memory(Data_Queue.total_size);
  Data_Queue.row = new_memory(Data_Queue.total_Qrows * sizeof(char *));
  rowptr = Data_Queue.raw;
  for ( i = 0; i < Data_Queue.total_Qrows; i++ ) {
    Data_Queue.row[i] = rowptr;
    rowptr += Data_Queue.nbQrow;
  }
  
  queue_tstamp( &tm_info.t_stmp );
  unlock_dq();
}


/* do_read_reply handles the actual reply after the message
   has been packed into iov_t structs. It may allocate a
   partial buffer if the request size is smaller than the
   message size. read_reply() may consult the request size
   to decide how large a message to return, of course, but
   the request size may be smaller than the smallest
   message.
*/
static void do_read_reply( RESMGR_OCB_T *ocb, int nb,
			iov_t *iov, int n_parts ) {
  int nreq = ocb->rw.read.nbytes;
  if ( nreq < nb ) {
    int i;
    char *p;
    
    if ( ocb->rw.read.buf == 0 )
      ocb->rw.read.buf = new_memory(Data_Queue.pbuf_size);
    assert( nb <= Data_Queue.pbuf_size );
    p = ocb->rw.read.buf;
    for ( i = 0; i < n_parts; i++ ) {
      int len = GETIOVLEN( iov[i] );
      memcpy( p, GETIOVBASE( iov[i] ), len );
      p += len;
    }
    ocb->part.dptr = ocb->rw.read.buf;
    ocb->part.ndata = nb;
    MsgReply( ocb->rw.read.rcvid, nreq, ocb->part.dptr, nreq );
    ocb->part.dptr += nreq;
    ocb->part.ndata -= nreq;
  } else {
    MsgReplyv( ocb->rw.read.rcvid, nb, iov, n_parts );
  }
}

/* read_reply(ocb) is called when we know we have
   something to return on a read request.
   First determine either the largest complete record that is less
   than the requested size or the smallest complete record. If the
   smallest record is larger than the request size, allocate the
   partial buffer and copy the record into it.
   Partial buffer size, if allocated, should be the larger of the size
   of the tm_info message or a one-row data message (based on the
   assumption that if a small request comes in, the smallest full
   message will be chosen for output)

   It is assumed that ocb has already been removed from whatever wait
   queue it might have been on.
*/
static void read_reply( RESMGR_OCB_T *ocb, int nonblock ) {
  iov_t iov[3];
  int nb;
  
  if ( ocb->part.nbdata ) {
    nb = ocb->rw.read.nbytes;
    if ( ocb->part.nbdata < nb )
      nb = ocb->part.nbdata;
    MsgReply( ocb->rw.read.rcvid, nb, ocb->part.dptr, nb );
    ocb->part.dptr += nb;
    ocb->nbdata -= nb;
    ocb->off += nb; //### Candidate for deletion
    // New state was already defined (or irrelevant)
  } else if ( ocb->data.dqd == 0 ) {
    lock_dq();
    if ( DQD_Queue.first ) {
      ocb->data.dqd = DQD_Queue.first;
      ocb->data.n_Qrows = 0;
    
      ocb->state = TM_STATE_HDR; //### delete
      ocb->part.hdr.s.hdr.tm_id = TMHDR_WORD;
      ocb->part.hdr.s.hdr.tm_type = TMTYPE_INIT;

      // Message consists of
      //   tm_hdr_t (TMHDR_WORD, TMTYPE_INIT)
      //   tm_info_t with the current timestamp
      SETIOV( &iov[0], &ocb->part.hdr.s.hdr,
	sizeof(ocb->part.hdr.s.hdr) );
      SETIOV( &iov[1], &tm_info, sizeof(tm_info)-sizeof(tstamp_t) );
      SETIOV( &iov[2], &ocb->data.dqd->TSq->TS, sizeof(tstamp_t) );
      nb = sizeof(tm_hdr_t) + sizeof(tm_info_t);
      do_read_reply( ocb, nb, iov, 3 );
    } else enqueue_read( ocb, nonblock );
    unlock_dq();
  } else {
    /* I've handled ocb->data.n_Qrows */
    dq_descriptor *dqd = ocb->data.dqd;

    lock_dq();
    while (dqd) {
      int nQrows_ready;
      
      /* DQD has a total of dqd->Qrows_expired + dqd->n_Qrows */
      if ( ocb->data.n_Qrows < dqd->Qrows_expired ) {
	// then we've missed some data: make a note and set
	ocb->rw.read.rows_missing +=
	  dqd->Qrows_expired - ocb->data.n_Qrows;
	ocb->data.n_Qrows = dqd->Qrows_expired;
      }
      nQrows_ready = dqd->n_Qrows + dqd->Qrows_expired
		      - ocb->data.n_Qrows;
      assert( nQrows_ready >= 0 );
      if ( nQrows_ready > 0 ) {
	if ( dqd->next == 0 && nQrows_ready < ocb->rw.read.maxQrows
	     && ocb->hdr.attr->node_type == TM_DCo && !nonblock ) {
	  enqueue_read( ocb, nonblock );
	} else {
	  int XRow_Num, NMinf, Row_Num_start, n_iov;
	  mfc_t MFCtr_start;
	  int Qrow_start, nQ1, nQ2;
	  
	  if ( nQrows_ready > ocb->rw.read.maxQrows )
	    nQrows_ready = ocb->rw.read.maxQrows;
	  ocb->part.hdr.s.hdr.tm_id = TMHDR_WORD;
	  ocb->part.hdr.s.hdr.tm_type = Data_Queue.output_tm_type;
	  ocb->part.hdr.s.u.dhdr.n_rows = nQrows_ready;
	  XRow_Num = dqd->Row_num + ocb->data.nQrows;
	  NMinf = XRow_Num/tmi(nrowminf);
	  MFCtr_start = dqd->MFCtr + NMinf;
	  Row_Num_start = XRow_Num % tmi(nrowminf);
	  switch ( Data_Queue.output_tm_type ) {
	    case TMTYPE_DATA_T1: break;
	    case TMTYPE_DATA_T2:
	      ocb->part.hdr.s.u.dhdr.mfctr = MFCtr_start;
	      ocb->part.hdr.s.u.dhdr.rownum = Row_Num_start;
	      break;
	    case TMTYPE_DATA_T3:
	      ocb->part.hdr.s.u.dhdr.mfctr = MFCtr_start;
	      break;
	    default:
	      nl_error(4,"Invalid output_tm_type" );
	  }
	  SETIOV( &iov[0], &ocb->part.hdr.s.hdr, Data_Queue.nbDataHdr );
	  Qrow_start = dqd->starting_Qrow + ocb->data.nQrows -
			  dqd->Qrows_expired;
	  if ( Qrow_start > Data_Queue.total_Qrows )
	    Qrow_start -= Data_Queue.total_Qrows;
	  nQ1 = nQrows_ready;
	  nQ2 = Qrow_start + nQ1 - Data_Queue.total_Qrows;
	  if ( nQ2 > 0 ) {
	    nQ1 -= nQ2;
	    SETIOV( &iov[2], dqd->row[0], nQ2 * Data_Queue.nbQrow );
	    n_iov = 3;
	  } else n_iov = 2;
	  SETIOV( &iov[1], dqd->row[Qrow_start],
		      nQ1 * Data_Queue.nbQrow );
	  do_read_reply( ocb,
	    Data_Queue.nbDataHdr + nQrows_ready * Data_Queue.nbQrow,
	    iov, n_iov );
	}
	break; // out of while(dqd)
      } else if ( dqd->next ) {
	int do_TS = dqd->TSq != dqd->next->TSq;
	dqd = dq_deref(dqd);
	dqd->ref_count++;
	ocb->data.dqd = dqd;
	ocb->data.n_Qrows = 0;
	if ( do_TS ) {
	  ocb->part.hdr.s.hdr.tm_id = TMHDR_WORD;
	  ocb->part.hdr.s.hdr.tm_type = TMTYPE_TSTAMP;
	  SETIOV( &iov[0], &ocb->part.hdr.s.hdr, sizeof(tm_hdr_t) );
	  SETIOV( &iov[1], &dqd->TSq->TS, sizeof(tstamp_t) );
	  do_read_reply( ocb, sizeof(tm_hdr_t)+sizeof(tstamp_t),
	    iov, 2 );
	  break;
	} // else loop through again
      } else {
	enqueue_read( ocb, nonblock );
	break;
      }
    }
    unlock_dq();
  }
}

static int io_read (resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb) {
  int status, nonblock = 0;
  IOFUNC_ATTR_T *handle = ocb->hdr.attr;

  if ((status = iofunc_read_verify( ctp, msg,
		     (iofunc_ocb_t *)ocb, &nonblock)) != EOK)
    return (status);
      
  if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE)
    return (ENOSYS);

  ocb->rw.read.rcvid = ctp->rcvid;
  ocb->rw.read.nbytes = msg->i.nbytes;
  if ( ocb->data.dqd ) {
    int nbData = ocb->rw.read.nbytes - Data_Queue.nbDataHdr;
    ocb->rw.read.maxQrows = nbData < Data_Queue.nbQrow ? 1 : nbData/Data_Queue.nbQrow;
  }
  read_reply( ocb, nonblock );
  return _RESMGR_NOREPLY;
}
