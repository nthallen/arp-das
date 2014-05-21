/* TMbfr2.c */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include "TMbfr.h"
#include "nortlib.h"
#include "nl_assert.h"
#include "oui.h"
#include "rundir.h"

static int io_read (resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb);
static int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb);
static int io_notify(resmgr_context_t *ctp, io_notify_t *msg, RESMGR_OCB_T *ocb);
static void do_write( IOFUNC_OCB_T *ocb, int nonblock, int new_rows );
static int io_open( resmgr_context_t *ctp, io_open_t *msg,
                    IOFUNC_ATTR_T *attr, void *extra );
static void read_reply( RESMGR_OCB_T *ocb, int nonblock );
static int process_tm_info( IOFUNC_OCB_T *ocb );
static int (*data_state_eval)( IOFUNC_OCB_T *ocb, int nonblock );
static int data_state_T1( IOFUNC_OCB_T *ocb, int nonblock );
static int data_state_T2( IOFUNC_OCB_T *ocb, int nonblock );
static int data_state_T3( IOFUNC_OCB_T *ocb, int nonblock );
static void queue_tstamp( tstamp_t *ts );
static dq_descriptor_t *new_dq_descriptor( TS_queue_t *TS );
static dq_descriptor_t *dq_deref( dq_descriptor_t *dqd, int use_next );
static dq_descriptor_t *dq_expire_check( dq_descriptor_t *dqd );
static void enqueue_read( IOFUNC_OCB_T *ocb, int nonblock );
static void run_read_queue(void);
static void lock_dq(void);
static void unlock_dq(void);
static int all_closed(void);
static int allocate_qrows( IOFUNC_OCB_T *ocb, int nrows_req, int nonblock );

static resmgr_connect_funcs_t    connect_funcs;
static resmgr_io_funcs_t         rd_io_funcs, wr_io_funcs;
static IOFUNC_ATTR_T             dg_attr, dcf_attr, dco_attr;
static int                       dg_id, dcf_id, dco_id;
static resmgr_attr_t             resmgr_attr;
static dispatch_t                *dpp;
static pthread_mutex_t           dg_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t           dq_mutex = PTHREAD_MUTEX_INITIALIZER;

static int dg_opened = 0;
data_queue_t Data_Queue;
DQD_Queue_t DQD_Queue;
static tm_ocb_t *blocked_writer;
static tm_ocb_t *all_readers;

#define LOG_EVENTS 0
#if LOG_EVENTS
  #define EVENT_LOG_SIZE 100
  static int my_events[EVENT_LOG_SIZE];
  static unsigned my_evt_num, my_evt_idx;
  static void log_event(int evt_id) {
    my_events[my_evt_idx++] = evt_id;
    if ( my_evt_idx >= EVENT_LOG_SIZE )
      my_evt_idx = 0;
    ++my_evt_num;
  }
#else
  #define log_event(x)
#endif

/* enqueue_read() queues the specified ocb.
   Assumes dq is locked.
   If nonblock is set, returns error to the client.
   Should prioritize by some clever scheme including priority, but
   for starters, I'll just FIFO.
 */
static void enqueue_read( IOFUNC_OCB_T *ocb, int nonblock ) {
  if ( dg_opened == 2 ) {
    MsgReply(ocb->rw.read.rcvid, 0, ocb->part.dptr, 0 );
  } else if ( nonblock ) {
    log_event(2);
    if ( MsgError( ocb->rw.read.rcvid, EAGAIN ) == -1 )
      nl_error( 2, "Error %d on MsgError", errno );
  } else {
    log_event(1);
    ocb->rw.read.blocked = 1;
  }
}

static void enqueue_reader( IOFUNC_OCB_T *ocb ) {
  lock_dq();
  ocb->next_ocb = all_readers;
  all_readers = ocb;
  unlock_dq();
}

static void dequeue_reader( IOFUNC_OCB_T *ocb ) {
  IOFUNC_OCB_T **rq;
  lock_dq();
  assert(all_readers != NULL);
  for ( rq = &all_readers; *rq != 0 && *rq != ocb; rq = &(*rq)->next_ocb );
  assert(*rq == ocb);
  *rq = ocb->next_ocb;
  unlock_dq();
}

static void run_write_queue(void) {
  if ( blocked_writer ) {
    log_event(3);
    int new_rows = data_state_eval(blocked_writer, 0);
    if ( blocked_writer->part.nbdata > 0 ) {
      log_event(4);
      do_write(blocked_writer, 0, new_rows);
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
  rq = all_readers;
  unlock_dq();
  log_event(5);
  while ( rq ) {
    if ( rq->rw.read.blocked ) {
      rq->rw.read.blocked = 0;
      log_event(6);
      read_reply(rq, 0);
    }
    rq = rq->next_ocb;
  }
  // if (IOFUNC_NOTIFY_INPUT_CHECK(dcf_attr.notify, 1, 0))
    iofunc_notify_trigger(dcf_attr.notify, 1, IOFUNC_NOTIFY_INPUT);
  // if (IOFUNC_NOTIFY_INPUT_CHECK(dco_attr.notify, 1, 0))
    iofunc_notify_trigger(dco_attr.notify, 1, IOFUNC_NOTIFY_INPUT);
  run_write_queue();
}

// Return the minimum number of rows processed by any reader that is currently
// referencing the specified dqd.  Assumes dq is locked.
// ### should mark the min readers as laggards
int min_reader( dq_descriptor_t *dqd ) {
  if ( dqd != DQD_Queue.first ) return 0;
  int min = dqd->Qrows_expired + dqd->n_Qrows;
  IOFUNC_OCB_T *ocb;
  for ( ocb = all_readers; ocb != 0; ocb = ocb->next_ocb ) {
    if ( ocb->data.dqd == dqd && ocb->data.n_Qrows < min )
      min = ocb->data.n_Qrows;
  }
  return min;
}

/* dq_deref() reduces reference count by one, does any work
   associated with ref count dropping to zero and returns the
   next dq_descriptor. dq must be locked.
 */
static dq_descriptor_t *dq_deref( dq_descriptor_t *dqd, int use_next ) {
  dq_descriptor_t *next_dqd = dqd->next;
  assert(dqd->ref_count > 0);
  if (next_dqd != 0 && use_next)
    ++next_dqd->ref_count;
  if ( --dqd->ref_count == 0 )
    dq_expire_check(dqd);
  return next_dqd;
}

static dq_descriptor_t *dq_expire_check( dq_descriptor_t *dqd ) {
  assert(dqd->ref_count >= 0);
  while ( dqd->ref_count == 0 && dqd->next != NULL
       && dqd->n_Qrows == 0 && DQD_Queue.first == dqd ) {
    /* Can expire this dqd */
    dq_descriptor_t *next_dqd = dqd->next;
    assert(dqd->TSq->ref_count >= 0);
    if ( --dqd->TSq->ref_count == 0 ) {
      free_memory(dqd->TSq);
    }
    free_memory(dqd);
    DQD_Queue.first = next_dqd;
    dqd = next_dqd;
  }
  return dqd;
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
  if (device->node_type == TM_DG ) {
    ocb->part.nbdata = sizeof(ocb->part.hdr); // ### and this
    ocb->part.dptr = (char *)&ocb->part.hdr; // ### and this
  } else {
    ocb->part.nbdata = 0;
    ocb->part.dptr = NULL;
    ocb->rw.read.blocked = 0;
    ocb->rw.read.ionotify = 0;
    enqueue_reader(ocb);
  }
  return ocb;
}

static void ocb_free(struct tm_ocb *ocb) {
  /* Be sure to remove this from the blocking list:
     Actually, there really is no way it should be on
     the blocking list. */
  // assert( ocb->rw.read.rcvid == 0 );
  // rcvid never gets reset
  lock_dq();
  if ( ocb->data.dqd != 0 )
    dq_deref(ocb->data.dqd, 0);
  unlock_dq();
  if (ocb->hdr.attr->node_type == TM_DG ) {
    dg_opened = 2;
    run_read_queue();
  } else {
    ocb->rw.read.blocked = 0;
    ocb->rw.read.ionotify = 0;
    if ( ocb->rw.read.buf ) free(ocb->rw.read.buf);
    dequeue_reader(ocb);
  }
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
         IOFUNC_ATTR_T *attr, resmgr_io_funcs_t *funcs ) {
  const char *server_name;
  int mnt_id;

  iofunc_attr_init((iofunc_attr_t *)attr, S_IFNAM | mode, 0, 0);
  attr->attr.nbytes = 0;
  attr->attr.mount = &mountpoint;
  attr->node_type = node_type;
  IOFUNC_NOTIFY_INIT(attr->notify);

  server_name = tm_dev_name( namebase );
  mnt_id = resmgr_attach(dpp,            /* dispatch handle        */
                     &resmgr_attr,   /* resource manager attrs */
                     server_name,    /* device name            */
                     _FTYPE_ANY,     /* open type              */
                     0,              /* flags                  */
                     &connect_funcs, /* connect routines       */
                     funcs,         /* I/O routines           */
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

// all_closed() returns non-zero if all OCBs are closed
static int all_closed(void) {
  return dg_attr.attr.count == 0 &&
         dcf_attr.attr.count == 0 &&
         dco_attr.attr.count == 0;
}

int main(int argc, char **argv ) {
  int use_threads = 0;

  oui_init_options( argc, argv );
  nl_error( 0, "Startup" );
  setup_rundir();
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
  rd_io_funcs.notify = io_notify;

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
        return 1;
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
  nl_error( 0, "Shutdown" );
  return 0;
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

int io_notify(resmgr_context_t *ctp, io_notify_t *msg, RESMGR_OCB_T *ocb) {
  tm_attr_t *dattr = (tm_attr_t *) ocb->hdr.attr;
  int trig;

  log_event(7);
  trig = _NOTIFY_COND_OUTPUT;         /* clients can always give us data */
  lock_dq();
  if ( dg_opened == 2 || ocb->part.nbdata )
    trig |= _NOTIFY_COND_INPUT;
  else if ( ocb->data.dqd == 0 ) {
    if ( DQD_Queue.first )
      trig |= _NOTIFY_COND_INPUT;
  } else {
    dq_descriptor_t *dqd = ocb->data.dqd;
    if ( dqd->next != 0)
      trig |= _NOTIFY_COND_INPUT;
    else {
      int nQrows_ready = dqd->n_Qrows;
      if ( ocb->data.n_Qrows > dqd->Qrows_expired )
        nQrows_ready -= ocb->data.n_Qrows - dqd->Qrows_expired;
      if ( nQrows_ready )
        trig |= _NOTIFY_COND_INPUT;
    }
  }
  if (trig & _NOTIFY_COND_INPUT) log_event(8);
  unlock_dq();
  return (iofunc_notify(ctp, msg, dattr->notify, trig, NULL, NULL));
}

// This is where data is recieved.
static int io_write( resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb ) {
  int status, nonblock;

  log_event(9);
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

  do_write( ocb, nonblock, 0 );
  return _RESMGR_NOREPLY;
}

// ### figure out how to get back into this routine after
// being blocked.
// Writer can only be blocked when processing data, since
// INIT and TSTAMP records can always be processed immediately.
// The data blocking can occur in either TM_STATE_HDR after
// we've looked at the header and decided we don't have room
// for the data or in TM_STATE_DATA after we've read some of
// the data from a message/record and run out of room.
// The trickiest part here is in the transition from TM_STATE_HDR
// to TM_STATE_DATA, we have to copy the surplus header info
// into the allocated space. If we can get allocate_qrows to
// do that based on the state (and update the state), then
// coming out of blocking involves calling data_state_eval,
// and then calling do_write if space is available.
static void do_write( IOFUNC_OCB_T *ocb, int nonblock, int new_rows ) {
  blocked_writer = 0;
  
  // _IO_SET_WRITE_NBYTES( ctp, msg->i.nbytes );
  // Not necessary since we'll handle the return ourselves
  // However, that means we need to store the total message size
  // Use off_msg for total size.
  
  // We loop here as long as we have work to do. We can get out
  // if we've processed all the data in the message (nb_msg == 0)
  // or exhausted all the available space in the DQ (nbdata == 0)
  while ( ocb->rw.write.nb_msg > 0 && ocb->part.nbdata > 0 ) {
    int nbread;
    nbread = ocb->rw.write.nb_msg < ocb->part.nbdata ?
      ocb->rw.write.nb_msg : ocb->part.nbdata;
    if ( MsgRead( ocb->rw.write.rcvid, ocb->part.dptr, nbread,
          ocb->rw.write.off_msg + sizeof(struct _io_write) ) < 0 ) {
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
      // ocb->rw.write.off_rec += nbread;
      ocb->rw.write.off_queue += nbread;
    }
    assert(ocb->part.nbdata >= 0);
    if ( ocb->part.nbdata == 0 ) {
      switch ( ocb->state ) {
        case TM_STATE_HDR:
          if ( ocb->part.hdr.s.hdr.tm_id != TMHDR_WORD ) {
            nl_error( 2, "Invalid Message header" );
            MsgError( ocb->rw.write.rcvid, EINVAL );
            ocb->part.nbdata = sizeof( ocb->part.hdr );
            ocb->part.dptr = (char *)&ocb->part.hdr;
            return;
          }
          switch ( ocb->part.hdr.s.hdr.tm_type ) {
            case TMTYPE_INIT:
              if ( DQD_Queue.last )
                nl_error( 3, "Second TMTYPE_INIT received" );
              ocb->rw.write.off_queue = sizeof(tm_hdrs_t)-sizeof(tm_hdr_t);
              ocb->part.nbdata = sizeof(tm_info) - ocb->rw.write.off_queue;
              ocb->part.dptr = (char *)&tm_info;
              memcpy( ocb->part.dptr, &ocb->part.hdr.raw[sizeof(tm_hdr_t)],
                      ocb->rw.write.off_queue );
              ocb->part.dptr += ocb->rw.write.off_queue;
              ocb->state = TM_STATE_INFO;
              break;
            case TMTYPE_TSTAMP:
              if ( DQD_Queue.last == 0 )
                nl_error( 3, "TMTYPE_TSTAMP received before _INIT" );
              lock_dq();
              queue_tstamp( &ocb->part.hdr.s.u.ts );
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
              if ( DQD_Queue.last == 0 )
                nl_error( 3, "Second TMTYPE_DATA* received before _INIT" );
              assert( data_state_eval != 0 );
              assert( ocb->part.hdr.s.hdr.tm_type == Data_Queue.output_tm_type );
              ocb->rw.write.nb_rec = ocb->part.hdr.s.u.dhdr.n_rows *
                ocb->rw.write.nbrow_rec;
              ocb->rw.write.off_queue = 0;
              new_rows += data_state_eval(ocb, nonblock);
              // ### Make sure data_state_eval returns the number
              // of rows that have been completely added to DQ
              if ( ocb->rw.write.nb_rec <= 0 ) {
                ocb->state = TM_STATE_HDR;
                ocb->part.nbdata = sizeof(ocb->part.hdr);
                ocb->part.dptr = (char *)&ocb->part.hdr;
              } // else break out
              break;
            default:
              nl_error( 4, "Invalid state" );
          }
          break;
        case TM_STATE_INFO:
          // ### Check return value
          process_tm_info(ocb);
          Data_Queue.nonblocking = 1; // nonblock;
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
    if ( new_rows ) run_read_queue();
    //### Mark us as not blocked: maybe that's nbdata != 0?
  } else {
    // We must have nbdata == 0 meaning we're going to block
    assert(!nonblock);
    blocked_writer = ocb;
    run_read_queue();
  }
}


// allocate_qrows() is responsible for finding space in the
// Data_Queue for incoming data. It returns the number of
// contiguous rows currently available starting at Data_Queue.last.
// If nonblock, allocate_qrows is guaranteed to return a
// non-zero value. If nonblock is zero and clients have not
// finished reading the oldest data in the queue, allocate_qrows
// will return zero.
// This routine assumes the dq is locked (lock_dq()).
static int allocate_qrows( IOFUNC_OCB_T *ocb, int nrows_req, int nonblock ) {
  int nrows_free;
  // dqd is used to find old data to expire, not for appending new data
  // Hence we start at the front of the queue, not the end.
  dq_descriptor_t *dqd = DQD_Queue.first;

  assert(nrows_req >= 0);
  // If we are nonblocking and are requesting more than total_Qrows, we are
  // going to expire rows from this transfer without ever committing them
  // to the queue. This may be handled implicitly by the while loop below.
  // This should of course never happen!
  if ( !Data_Queue.full && Data_Queue.last >= Data_Queue.first ) {
    nrows_free = Data_Queue.total_Qrows - Data_Queue.last;
    if ( nrows_free > nrows_req )
      nrows_free = nrows_req;
    // and we're done because any additional rows would be
    // non-contiguous
  } else {
    nrows_free = Data_Queue.first - Data_Queue.last;
    if ( nrows_free > nrows_req )
      nrows_free = nrows_req;
    nrows_req -= nrows_free;
    // if there is enough room, use it now, otherwise
    while ( nrows_req > 0 ) {
      int n_expire, opt_expire;
      while ( dqd->n_Qrows == 0 && dqd->next )
        dqd = dqd->next;
      assert( dqd->starting_Qrow == Data_Queue.first );
      // We can expire the minimum of:
      //   nrows_req,
      //   Data_Queue.total_Qrows - Data_Queue.last - nrows_free
      //   dqd->n_Qrows == all rows in dqd
      //   Data_Queue.total_Qrows - dqd->starting_Qrow(largest contiguous)
      //   dqd->min_reader - dqd->Qrows_expired
      // The third condition only applies if blocking
      // The min_reader method must return 0 if any prior dqd has
      // readers. We won't get here unless the prior dqds are empty,
      // so the only thing that would prevent the prior dqds from
      // expiring would be readers, so min_read will return 0 unless
      // this dqd is the first.
      n_expire = nrows_req;
      opt_expire = Data_Queue.total_Qrows - Data_Queue.last;
      opt_expire = (opt_expire >= nrows_free) ?
          opt_expire - nrows_free : 0;
      if ( opt_expire < n_expire ) n_expire = opt_expire;
      if ( dqd->n_Qrows < n_expire ) n_expire = dqd->n_Qrows;
      opt_expire = Data_Queue.total_Qrows - dqd->starting_Qrow;
      if ( opt_expire < n_expire ) n_expire = opt_expire;
      if ( !nonblock ) {
        int min_rdr = min_reader(dqd);
        if (min_rdr > dqd->Qrows_expired) {
          opt_expire = min_rdr - dqd->Qrows_expired;
          if ( opt_expire >= 0 && opt_expire < n_expire )
            n_expire = opt_expire;
        } else n_expire = 0;
      }
      assert(n_expire >= 0);
      if ( n_expire ) {
        dqd->starting_Qrow += n_expire;
        if ( dqd->starting_Qrow == Data_Queue.total_Qrows )
          dqd->starting_Qrow = 0;
        Data_Queue.first = dqd->starting_Qrow;
        Data_Queue.full = 0;
        dqd->n_Qrows -= n_expire;
        dqd->Qrows_expired += n_expire;
        nrows_free += n_expire;
        nrows_req -= n_expire;
        assert(Data_Queue.last+nrows_free <= Data_Queue.total_Qrows);
        if ( dqd->n_Qrows == 0 && dqd->next )
          dqd = dq_expire_check(dqd);
        else break;
      } else break;
    }
  }
  assert( dqd->n_Qrows == 0 || dqd->starting_Qrow == Data_Queue.first );
  assert(nrows_free >= 0);
  assert(Data_Queue.last+nrows_free <= Data_Queue.total_Qrows);
  ocb->part.nbdata = nrows_free * Data_Queue.nbQrow;
  ocb->part.dptr = Data_Queue.row[Data_Queue.last];
  ocb->rw.write.off_queue = 0;
  if ( nrows_free ) {
    switch ( ocb->state ) {
      case TM_STATE_HDR:
        // ### We need to move hdr data into Data_Queue?
        // sizeof(tm_hdrs_t) is how much we've read into ocb->part.hdr
        // ocb->rw.write.nbhdr_rec is how much is actually header
        // The rest is TM data that needs to be copied into the DQ
        ocb->rw.write.off_queue = sizeof(tm_hdrs_t) - ocb->rw.write.nbhdr_rec;
        // This could actually happen, but it shouldn't
        assert( ocb->rw.write.off_queue <= ocb->part.nbdata );
        ocb->part.nbdata -= ocb->rw.write.off_queue;
        memcpy( ocb->part.dptr,
                &ocb->part.hdr.raw[ocb->rw.write.nbhdr_rec],
                ocb->rw.write.off_queue );
        ocb->part.dptr += ocb->rw.write.off_queue;
        ocb->rw.write.nb_rec -= ocb->rw.write.off_queue;
        ocb->state = TM_STATE_DATA;
        break;
      case TM_STATE_DATA:
        break;
      default:
        nl_error( 4, "Invalid state in allocate_qrows" );
    }
  }
  return nrows_free;
}

// The job of data_state_eval is to decide how big the next
// chunk of data should be and where it should go.
// This involves finding space in the data queue and
// setting part.dptr and part.nbdata. We may need to expire
// rows from the data queue, but if we aren't nonblocking, we
// might block to allow the readers to handle what we've already
// got.
// data_state_eval() does not transfer any data via ReadMessage,
// but allocate_qrows() will copy any bytes from the hdr if
// ocb->state == TM_STATE_HDR
//
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
//   T2->T2 Copy straight in, then verify continuity with previous
// records.
//   T3->T3 Copy straight in. Verify consecutive, etc.

static int data_state_T1( IOFUNC_OCB_T *ocb, int nonblock ) {
  int nrowsfree, tot_nrrecd = 0;
  lock_dq();
  do {
    int nrrecd = ocb->rw.write.off_queue/Data_Queue.nbQrow;
    nrowsfree = 0;
    assert(ocb->part.nbdata == 0);
    assert(ocb->rw.write.off_queue == nrrecd*Data_Queue.nbQrow);
    if ( nrrecd ) {
      Data_Queue.last += nrrecd;
      assert(Data_Queue.last <= Data_Queue.total_Qrows );
      if ( Data_Queue.last == Data_Queue.total_Qrows )
        Data_Queue.last = 0;
      DQD_Queue.last->n_Qrows += nrrecd;
      ocb->rw.write.off_queue = 0;
      tot_nrrecd += nrrecd;
      // Don't need to update dptr (and can't anyway!) because nbdata==0
      // allocate_qrows will update nbdata, dptr and off_queue.
    }
    // Now look at what we have yet to move.
    if ( ocb->rw.write.nb_rec ) {
      int nrows = ocb->rw.write.nb_rec/ocb->rw.write.nbrow_rec;
      if ( nrows == 0 ) ++nrows;
      nrowsfree = allocate_qrows( ocb, nrows, nonblock );
      // Handle the unlikely case that an entire row arrived in the
      // 4 bytes of excess header. This can only happen if the row
      // consisted entirely of MFCtr and Synch.
      // if/when this happens, nrowsfree will be non-zero, but nbdata
      // may be zero
    }
  } while ( nrowsfree && ocb->part.nbdata == 0 );
  unlock_dq();
  return tot_nrrecd;
}

static int data_state_T2( IOFUNC_OCB_T *ocb, int nonblock ) {
  // int nrrecd = ocb->rw.write.off_queue/ocb->rw.write.nbrow_rec;
  assert(ocb->part.nbdata == 0);
  nl_error(4, "Not implemented");
  return 0;
}

static int data_state_T3( IOFUNC_OCB_T *ocb, int nonblock ) {
  // int nrrecd = ocb->rw.write.off_queue/ocb->rw.write.nbrow_rec;
  // assert(ocb->part.nbdata == 0); // redundant here: checked again in the loop below
  int nrowsfree, tot_nrrecd = 0;
  lock_dq();
  do {
    int nrrecd = ocb->rw.write.off_queue/Data_Queue.nbQrow;
    nrowsfree = 0;
    assert(ocb->part.nbdata == 0);
    assert(ocb->rw.write.off_queue == nrrecd*Data_Queue.nbQrow);
    if ( nrrecd ) {
      Data_Queue.last += nrrecd;
      assert(Data_Queue.last <= Data_Queue.total_Qrows );
      if ( Data_Queue.last == Data_Queue.total_Qrows )
        Data_Queue.last = 0;
      if ( Data_Queue.last == Data_Queue.first )
        Data_Queue.full = 1;
      DQD_Queue.last->n_Qrows += nrrecd;
      ocb->rw.write.off_queue = 0;
      tot_nrrecd += nrrecd;
      // Don't need to update dptr (and can't anyway!) because nbdata==0
      // allocate_qrows will update nbdata, dptr and off_queue.
    }
    // Now look at what we have yet to move.
    if ( ocb->rw.write.nb_rec ) {
      int nrows = ocb->rw.write.nb_rec/ocb->rw.write.nbrow_rec;
      if ( nrows == 0 ) nrows++;
      if ( nrows == ocb->part.hdr.s.u.dhdr.n_rows ) {
        dq_descriptor_t *dqd = DQD_Queue.last;
        if (dqd->MFCtr + dqd->Qrows_expired + dqd->n_Qrows !=
            ocb->part.hdr.s.u.dhdr.mfctr) {
          dqd = new_dq_descriptor(NULL);
          dqd->MFCtr = ocb->part.hdr.s.u.dhdr.mfctr;
        }
      }
      nrowsfree = allocate_qrows( ocb, nrows, nonblock );
      // Handle the case that an entire row arrived in the
      // 2 bytes of excess header. This could happen.
    }
  } while ( nrowsfree && ocb->part.nbdata == 0 );
  unlock_dq();
  return tot_nrrecd;
}

/* queue_tstamp(): Create a new tstamp record in the Tstamp_Queue
   and a new dq_descriptor that references it.
   This currently assumes the dq_mutex is locked.
*/
static void queue_tstamp( tstamp_t *ts ) {
  TS_queue_t *new_TS = new_memory(sizeof(TS_queue_t));
  dq_descriptor_t *new_dqd;
  
  new_TS->TS = *ts;
  new_TS->ref_count = 0;

  new_dqd = new_dq_descriptor( new_TS );
}

static dq_descriptor_t *new_dq_descriptor( TS_queue_t *TS ) {
  dq_descriptor_t *new_dqd = new_memory(sizeof(dq_descriptor_t));

  if ( TS == NULL ) TS = DQD_Queue.last->TSq;
  TS->ref_count++;

  new_dqd->next = 0;
  new_dqd->ref_count = 0;
  new_dqd->starting_Qrow = Data_Queue.last;
  new_dqd->n_Qrows = 0;
  new_dqd->Qrows_expired = 0;
  new_dqd->TSq = TS;
  new_dqd->MFCtr = 0;
  new_dqd->Row_num = 0;
  if ( DQD_Queue.last )
    DQD_Queue.last->next = new_dqd;
  else DQD_Queue.first = new_dqd;
  DQD_Queue.last = new_dqd;
  return new_dqd;
}

/* As soon as tm_info has been received, we can decide what
   data format to output, how much buffer space to allocate
   and in what configuration. We can then create the first
   timestamp record (with the TS in the tm_info) and the
   first dq_descriptor, albeit with no Qrows, but refrencing
   the the first timestamp. Then we can check to see if any
   readers are waiting, and initialize them.
*/
static int process_tm_info( IOFUNC_OCB_T *ocb ) {
  char *rowptr;
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
      tmi(nbminf) % tmi(nbrow) != 0 ||
      tm_info.nrowminf != tmi(nbminf)/tmi(nbrow)) {
    nl_error( 2, "Sanity Checks failed on incoming stream" );
    return EINVAL;
  }

  // What data format should we output?
  lock_dq();
  ocb->rw.write.buf = NULL;
  Data_Queue.nbQrow = tmi(nbrow);
  if ( tmi(mfc_lsb) == 0 && tmi(mfc_msb) == 1
       && tm_info.nrowminf == 1 ) {
    Data_Queue.output_tm_type = TMTYPE_DATA_T3;
    Data_Queue.nbQrow -= 4;
    Data_Queue.nbDataHdr = TM_HDR_SIZE_T3;
		ocb->rw.write.nbhdr_rec = TM_HDR_SIZE_T3;
		ocb->rw.write.nbrow_rec = tmi(nbrow) - 4;
		data_state_eval = data_state_T3;
  } else if ( tm_info.nrowminf == 1 ) {
    Data_Queue.output_tm_type = TMTYPE_DATA_T1;
    Data_Queue.nbDataHdr = TM_HDR_SIZE_T1;
		ocb->rw.write.nbhdr_rec = TM_HDR_SIZE_T1;
		data_state_eval = data_state_T1;
    if ( tmi(nbrow) <= 4 )
      nl_error( 3, "TM Frame with no non-synch data not supported" );
		ocb->rw.write.nbrow_rec = tmi(nbrow);
		data_state_eval = data_state_T1;
  } else {
    Data_Queue.output_tm_type = TMTYPE_DATA_T2;
    Data_Queue.nbDataHdr = TM_HDR_SIZE_T2;
		ocb->rw.write.nbhdr_rec = TM_HDR_SIZE_T2;
		ocb->rw.write.nbrow_rec = tmi(nbrow);
		data_state_eval = data_state_T2;
  }
  Data_Queue.pbuf_size = Data_Queue.nbDataHdr + Data_Queue.nbQrow;
  if ( Data_Queue.pbuf_size < sizeof(tm_hdr_t) + sizeof(tm_info_t) )
    Data_Queue.pbuf_size = sizeof(tm_hdr_t) + sizeof(tm_info_t);

  // how much buffer space to allocate?
  // Let's default to one minute's worth, but make sure we get
  // an integral number of minor frames
  Data_Queue.total_Qrows = tm_info.nrowminf *
    ( ( tmi(nrowsper) * 60 + tmi(nsecsper)*tm_info.nrowminf - 1 )
        / (tmi(nsecsper)*tm_info.nrowminf) );
  Data_Queue.total_size =
    Data_Queue.nbQrow * Data_Queue.total_Qrows;
  Data_Queue.first = Data_Queue.last = Data_Queue.full = 0;
  Data_Queue.raw = new_memory(Data_Queue.total_size);
  Data_Queue.row = new_memory(Data_Queue.total_Qrows * sizeof(char *));
  rowptr = Data_Queue.raw;
  for ( i = 0; i < Data_Queue.total_Qrows; i++ ) {
    Data_Queue.row[i] = rowptr;
    rowptr += Data_Queue.nbQrow;
  }
  
  queue_tstamp( &tm_info.t_stmp );
  unlock_dq();
  return 0;
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
      int len = GETIOVLEN( &iov[i] );
      memcpy( p, GETIOVBASE( &iov[i] ), len );
      p += len;
    }
    ocb->part.dptr = ocb->rw.read.buf;
    ocb->part.nbdata = nb;
    MsgReply( ocb->rw.read.rcvid, nreq, ocb->part.dptr, nreq );
    ocb->part.dptr += nreq;
    ocb->part.nbdata -= nreq;
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
    ocb->part.nbdata -= nb;
    // New state was already defined (or irrelevant)
  } else if ( ocb->data.dqd == 0 ) {
    lock_dq();
    if ( DQD_Queue.first ) {
      ocb->data.dqd = DQD_Queue.first;
      ++ocb->data.dqd->ref_count;
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
    dq_descriptor_t *dqd = ocb->data.dqd;

    lock_dq();
    while (dqd) {
      int nQrows_ready;
      
      /* DQD has a total of dqd->Qrows_expired + dqd->n_Qrows */
      if ( ocb->data.n_Qrows < dqd->Qrows_expired ) {
        // then we've missed some data: make a note and set
        int n_missed = dqd->Qrows_expired - ocb->data.n_Qrows;
        ocb->rw.read.rows_missing += n_missed;
        ocb->data.n_Qrows = dqd->Qrows_expired;
      }
      nQrows_ready = dqd->n_Qrows + dqd->Qrows_expired
                      - ocb->data.n_Qrows;
      assert( nQrows_ready >= 0 );
      if ( nQrows_ready > 0 ) {
        // ### make sure rw.read.maxQrows < Data_Queue.total_Qrows
        // (it was not! fixed in io_read().
        if ( blocked_writer == 0 && dg_opened < 2 &&
             dqd->next == 0 && nQrows_ready < ocb->rw.read.maxQrows
             && ocb->hdr.attr->node_type == TM_DCo && !nonblock ) {
          // We want to wait for more
          // ### reasons not to wait:
          // ###   DG is blocked or has terminated
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
          XRow_Num = dqd->Row_num + ocb->data.n_Qrows;
          NMinf = XRow_Num/tm_info.nrowminf;
          MFCtr_start = dqd->MFCtr + NMinf;
          Row_Num_start = XRow_Num % tm_info.nrowminf;
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
          Qrow_start = dqd->starting_Qrow + ocb->data.n_Qrows -
                          dqd->Qrows_expired;
          if ( Qrow_start > Data_Queue.total_Qrows )
            Qrow_start -= Data_Queue.total_Qrows;
          nQ1 = nQrows_ready;
          nQ2 = Qrow_start + nQ1 - Data_Queue.total_Qrows;
          if ( nQ2 > 0 ) {
            nQ1 -= nQ2;
            SETIOV( &iov[2], Data_Queue.row[0], nQ2 * Data_Queue.nbQrow );
            n_iov = 3;
          } else n_iov = 2;
          SETIOV( &iov[1], Data_Queue.row[Qrow_start],
                      nQ1 * Data_Queue.nbQrow );
          ocb->data.n_Qrows += nQrows_ready;
          do_read_reply( ocb,
            Data_Queue.nbDataHdr + nQrows_ready * Data_Queue.nbQrow,
            iov, n_iov );
        }
        break; // out of while(dqd)
      } else if ( dqd->next ) {
        int do_TS = dqd->TSq != dqd->next->TSq;
        dqd = dq_deref(dqd, 1);
        // dqd->ref_count++;
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
  // IOFUNC_ATTR_T *handle = ocb->hdr.attr;

  log_event(10);
  if ((status = iofunc_read_verify( ctp, msg,
                     (iofunc_ocb_t *)ocb, &nonblock)) != EOK)
    return (status);
      
  if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE)
    return (ENOSYS);

  ocb->rw.read.rcvid = ctp->rcvid;
  ocb->rw.read.nbytes = msg->i.nbytes;
  if ( ocb->data.dqd ) {
    int nbData = ocb->rw.read.nbytes - Data_Queue.nbDataHdr;
    ocb->rw.read.maxQrows =
      nbData < Data_Queue.nbQrow ? 1 : nbData/Data_Queue.nbQrow;
    if ( ocb->rw.read.maxQrows > Data_Queue.total_Qrows )
      ocb->rw.read.maxQrows = Data_Queue.total_Qrows;
  }
  read_reply( ocb, nonblock );
  run_write_queue();
  return _RESMGR_NOREPLY;
}
