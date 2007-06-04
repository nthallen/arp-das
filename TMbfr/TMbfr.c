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

static resmgr_connect_funcs_t    connect_funcs;
static resmgr_io_funcs_t         rd_io_funcs, wr_io_funcs;
static IOFUNC_ATTR_T             dg_attr, dcf_attr, dco_attr;
static int                       dg_id, dcf_id, dco_id;
static char *                    dg_name, dcf_name, dco_name;
static resmgr_attr_t             resmgr_attr;
static dispatch_t                *dpp;
static pthread_mutex_t           dg_mutex = PTHREAD_MUTEX_INITIALIZER;

static tm_ocb_t *ocb_calloc(resmgr_context_t *ctp, IOFUNC_ATTR_T *device) {
  tm_ocb_t *ocb = calloc( 1, sizeof(tm_ocb_t) );
  if ( ocb == 0 ) return 0;
  /* Initialize any other elements. Currently all zeros is good. */
  ocb->next_ocb = 0;
  ocb->state = TM_STATE_HDR;
  ocb->part.buf = 0;
  ocb->part.nbhdr = 0;
  ocb->part.nbdata = 0;
  ocb->part.off = 0;
  ocb->data.dq = 0;
  ocb->data.n_Qrows = 0;
  ocb->read.rows_missing = 0;
  return ocb;
}

static void ocb_free(struct ocb *ocb) {
  /* Be sure to remove this from the blocking list:
     Actually, there really is no way it should be on
     the blocking list. */
  assert( ocb->read.rcvid == 0 );
  assert( ocb->next_ocb == 0 );
  dq_deref(ocb->data.dq);
  if ( ocb.part.buf ) free(ocb.part.buf);
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
  int status, msgsize, msgoffset;
  char buf[CMD_MAX_COMMAND_IN+1];

  status = iofunc_write_verify(ctp, msg, (iofunc_ocb_t *)ocb, NULL);
  if ( status != EOK )
    return status;

  if ((msg->i.xtype &_IO_XTYPE_MASK) != _IO_XTYPE_NONE )
    return ENOSYS;

  // iofunc_write_verify should have stopped us if this is a DC node,
  // since they have no write permissions, but just check to make sure.
  assert( ocb->hdr.attr->node_type == TM_DG );

  msgsize = msg->i.nbytes;
  msgoffset = sizeof(msg->i);

  _IO_SET_WRITE_NBYTES( ctp, msg->i.nbytes );
  
  while ( msgsize > 0 ) {
    switch ( ocb->state ) {
      case TM_STATE_HDR:
	{
	  int nbread;
	  nbread = sizeof(ocb->part.hdr) - ocb->part.off;
	  if ( msgsize < nbread ) nbread = msgsize;
	  resmgr_msgread( ctp, ocb->part.hdr.raw + ocb->part.off, nbread, msgoffset );
	  ocb->part.off += nbread;
	  msgsize -= nbhdr;
	  if ( ocb->part.off == sizeof(ocb->part.hdr) ) {
	    if ( ocb->part.hdr.s.hdr.tm_id != TMHDR_WORD )
	      return EINVAL;
	    switch ( ocb->part.hdr.s.hdr.tm_type ) {
	      case TMTYPE_INIT: ####
	      case TMTYPE_TSTAMP:
	      case TMTYPE_DATA_T1:
	      case TMTYPE_DATA_T2:
	      case TMTYPE_DATA_T3:
	      case TMTYPE_DATA_T4:
	      default:
	    }
	  }
	}
      case TM_STATE_INFO:
      case TM_STATE_DATA:
    }
  }

  resmgr_msgread( ctp, buf, msgsize, sizeof(msg->i) );
  buf[msgsize] = '\0';

  // Parse leading options
  // No spaces, colons or right brackets allowed in mnemonics 
  { char *mnemonic = "--";
    int quiet = 0;
    int testing = 0;
    char *s = buf;

    if ( *s == '[' ) {
      s++;
      if ( isgraph(*s) && *s != ':' && *s != ']' ) {
	mnemonic = s++;
	while ( isgraph(*s) && *s != ':' && *s != ']' )
	  s++;
      }
      if ( !isgraph(*s) ) {
	nl_error( 2, "Invalid mnemonic string" );
	return EINVAL;
      }
      if ( *s == ':' ) {
	int end_of_opts = 0;
	char *ver;

	*s++ = '\0'; // terminate the mnemonic
	// and then handle the options
	while (!end_of_opts) {
	  switch (*s) {
	    case 'T': testing = 1; s++; break;
	    case 'Q': quiet = 1; s++; break;
	    case 'X': process_quit(); return EOK;
	    case 'V': // handle version command
	      ver = ++s;
	      while ( *s != ']' && *s != '\0' ) s++;
	      if ( *s == '\0' ) {
		nl_error( 2, "Unterminated version string" );
		return EINVAL;
	      }
	      *s = '\0';
	      if ( strcmp( ver, ci_version ) == 0 )
		return EOK;
	      nl_error( 2, "Command Versions don't match" );
	      return EINVAL;
	    case ']': end_of_opts = 1; break;
	    default:
	      nl_error( 2, "Invalid option" );
	      return EINVAL;
	  }
	}
      }
      // blank out trailing ']' in case it's the end of the mnemonic
      *s++ = '\0';
    }
    { char *cmd = s;
      int len = 0;
      int rv;
      int clen;

      // Now s points to a command we want to parse.
      // Make sure it's kosher
      while ( *s ) {
	if ( ! isprint(*s) && *s != '\n' ) {
	  nl_error( 2, "Invalid character in command" );
	  return EINVAL;
	}
	len++;
	s++;
      }
      if ( len > 0 && cmd[len-1] == '\n' ) len--;
      nl_error( quiet ? -2 : 0, "%s: %*.*s",
	mnemonic, len, len, cmd );
      cmd_init();
      rv = cmd_batch( cmd, testing );
      ocb->hdr.attr->attr.flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;
      switch ( CMDREP_TYPE(rv) ) {
	case 0: return EOK;
	case 1: process_quit(); return ENOENT;
	case 2: /* Report Syntax Error */
	  if ( nl_response ) {
	    nl_error( 2, "%s: Syntax Error", mnemonic );
	    nl_error( 2, "%*.*s", len, len, cmd);
	    nl_error( 2, "%*s", rv - CMDREP_SYNERR, "^");
	  }
	  return EINVAL;
	default: return EIO;
      }
    }
  }
}

static void read_reply( RESMGR_OCB_T *ocb ) {
  int nb = ocb->nbytes_requested;
  command_out_t *cmd = ocb->next_command;
  int cmdbytes = cmd->cmdlen - ocb->hdr.offset;
  int bytes_returned = nb > cmdbytes ? cmdbytes : nb;

  assert(cmd->ref_count > 0);
  assert(cmdbytes >= 0);
  MsgReply( ocb->rcvid, bytes_returned,
    cmd->command + ocb->hdr.offset, bytes_returned );
  ocb->rcvid = 0;
  if ( bytes_returned < cmdbytes ) {
    ocb->hdr.offset += bytes_returned;
  } else {
    IOFUNC_ATTR_T *handle = ocb->hdr.attr;
    ocb->hdr.offset = 0;
    cmd->ref_count--;
    ocb->next_command = cmd->next;
    ocb->next_command->ref_count++;
    if ( handle->first_cmd->ref_count == 0 &&
	 handle->first_cmd->next != 0 ) {
      handle->first_cmd = free_command( handle->first_cmd );
    }
  }
}

static int io_read (resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb) {
  int status, nonblock = 0;
  IOFUNC_ATTR_T *handle = ocb->hdr.attr;

  if ((status = iofunc_read_verify( ctp, msg,
		     (iofunc_ocb_t *)ocb, NULL)) != EOK)
    return (status);
      
  if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE)
    return (ENOSYS);

  ocb->rcvid = ctp->rcvid;
  ocb->nbytes_requested = msg->i.nbytes;
  if ( ocb->next_command->cmdlen > ocb->hdr.offset ) {
    // we've got something to send now
    read_reply( ocb );
  } else if (nonblock) {
    ocb->rcvid = 0;
    return EAGAIN;
  } else {
    // Nothing at the moment.
    ocb->next_ocb = handle->blocked;
    handle->blocked = ocb;
  }
  return _RESMGR_NOREPLY;
}

static command_out_t *free_commands;

static command_out_t *new_command(void) {
  command_out_t *cmd;
  if ( free_commands ) {
    cmd = free_commands;
    free_commands = cmd->next;
  } else {
    cmd = new_memory(sizeof(command_out_t));
  }
  cmd->next = NULL;
  cmd->ref_count = 0;
  cmd->command[0] = '\0';
  cmd->cmdlen = 0;
  return cmd;
}

// Returns the next command so it's easy to free the
// first command in a list:
// list = free_command( list );
static command_out_t *free_command( command_out_t *cmd ) {
  command_out_t *nxt;
  assert( cmd != NULL );
  assert( cmd->ref_count == 0 );
  nxt = cmd->next;
  cmd->next = free_commands;
  free_commands = cmd;
  return nxt;
}

void cis_turf( IOFUNC_ATTR_T *handle, char *format, ... ) {
  va_list arglist;
  command_out_t *cmd;
  int nb;

  assert( handle != NULL );
  cmd = handle->last_cmd;
  va_start( arglist, format );
  nb = vsnprintf( cmd->command, CMD_MAX_COMMAND_OUT, format, arglist );
  va_end( arglist );
  if ( nb >= CMD_MAX_COMMAND_OUT ) {
    nl_error( 2, "Output buffer overflow to node %s", handle->nodename );
    cmd->command[0] = '\0';
  } else {
    cmd->cmdlen = nb;
    cmd->next = handle->last_cmd = new_command();
    // Now run the queue
    while ( handle->blocked ) {
      IOFUNC_OCB_T *ocb = handle->blocked;
      handle->blocked = ocb->next_ocb;
      ocb->next_ocb = NULL;
      assert(ocb->hdr.offset == 0);
      read_reply(ocb);
    }
  }
}

/*
=Name ci_server(): Command Server main loop
=Subject Command Server and Client

=Synopsis

#include "tm.h"
void ci_server(void);

=Description

ci_server() does all the work for a command server. It does
not return until =cmd_batch=() returns a CMDREP_QUIT or it receives
a CMDINTERP_QUIT message.<P>

It registers the CMDINTERP_NAME with the operating system, then
loops to Receive messages. For each received command, ci_server()
calls =cmd_init=() and =cmd_batch=(). If needed, a hook could
be added to Receive other messages.

=Returns
Nothing.

=SeeAlso
=Command Server and Client= functions.

=End

=Name cmd_batch(): Parse a command in batch mode
=Subject Command Server and Client
=Name cmd_init(): Initialize command parser
=Subject Command Server and Client
=Synopsis

#include "cmdalgo.h"
int cmd_batch( char *cmd, int test );
void cmd_init( void );
void cis_initialize(void);
void cis_terminate(void);
void cis_interfaces(void);

=Description

  These functions are all generated by CMDGEN. cmd_init()
  resets the parser to its start state. cmd_batch() provides an
  input string that the parser will act on. If test is non-zero,
  no actions associated with the command will be executed.
  cis_initialize() is called at the very beginning of ci_server(),
  and cis_terminate() is called at the very end. These are used
  by cmdgen and soldrv to handle proxy-based commands in QNX4.
  It remains to be seen if these will be used in QNX6.
  cis_interfaces() is called from ci_server() to initialize
  reader interfaces in QNX6.

=Returns

  cmd_batch() returns the same error codes as =ci_sendcmd=().

=SeeAlso

  =Command Server and Client= functions.

=End

*/
