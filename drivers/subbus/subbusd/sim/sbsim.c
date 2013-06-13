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
#include "subbusd_int.h"
#include "nl_assert.h"

static int n_writes = 0;
static int n_reads = 0;

static void ErrorReply( int rcvid, int rv ) {
  subbusd_rep_hdr_t rep;
  nl_assert( rv > 0 );
  rep.status = -rv;
  rep.ret_type = SBRT_NONE;
  rv = MsgReply( rcvid, sizeof(rep), &rep, sizeof(rep) );
}

/**
 This is where we serialize the request
 The basic sanity of the incoming message has been
 checked by subbus_io_msg before it gets here,
 so we can at least assume that the message was
 big enough to include the specified message type,
 and that the message type is defined.
 */
void incoming_sbreq( int rcvid, subbusd_req_t *req ) {
  subbusd_rep_t rep;
  int rv, rsize;
  unsigned short fail_val = 0;
  
  switch ( req->sbhdr.command ) {
    case SBC_READACK:
    case SBC_READCACHE:
      ++n_reads;
      rep.hdr.status =
        (sb_cache_read(req->data.d1.data, &rep.data.value) < 0) ?
        SBS_NOACK : SBS_ACK;
      rep.hdr.ret_type = SBRT_US;
      rsize =
        sizeof(subbusd_rep_hdr_t) + sizeof(unsigned short);    
      rv = MsgReply( rcvid, rsize, &rep, rsize );
      return;
    case SBC_WRITEACK:
    case SBC_WRITECACHE:
      ++n_writes;
      rv = sb_cache_write(req->data.d0.address, req->data.d0.data);
      rep.hdr.ret_type = SBRT_NONE;
      rep.hdr.status = (rv == 0) ? SBS_ACK : SBS_NOACK;
      rsize = sizeof(subbusd_rep_hdr_t);
      rv = MsgReply( rcvid, rsize, &rep, rsize );
      return;
    case SBC_GETCAPS:
      rep.hdr.ret_type = SBRT_CAP;
      rep.hdr.status = SBS_OK;
      rep.data.capabilities.subfunc = SB_SIM;
      rep.data.capabilities.features =
        SBF_SIC|SBF_SET_FAIL|SBF_READ_FAIL|SBF_READ_SW|
        SBF_CMDSTROBE;
      strncpy(rep.data.capabilities.name, "Subbus Simulator",
        SUBBUS_NAME_MAX);
      rsize = sizeof(subbusd_rep_hdr_t) + sizeof(subbusd_cap_t);
      rv = MsgReply( rcvid, rsize, &rep, rsize );
      return;
    case SBC_SETFAIL:
      fail_val = req->data.d1.data;
    case SBC_SETCMDENBL:
    case SBC_SETCMDSTRB:
    case SBC_TICK:
    case SBC_DISARM:
      rep.hdr.ret_type = SBRT_NONE;
      rep.hdr.status = SBS_ACK;
      rsize = sizeof(subbusd_rep_hdr_t);
      rv = MsgReply( rcvid, rsize, &rep, rsize );
      return;
    case SBC_READSW:
      rep.hdr.ret_type = SBRT_US;
      rep.hdr.status = SBS_ACK;
      rep.data.value = 0;
      rsize = sizeof(subbusd_rep_hdr_t) + sizeof(unsigned short);
      rv = MsgReply( rcvid, rsize, &rep, rsize );
      return;
    case SBC_READFAIL:
      rep.hdr.ret_type = SBRT_US;
      rep.hdr.status = SBS_ACK;
      rep.data.value = fail_val;
      rsize = sizeof(subbusd_rep_hdr_t) + sizeof(unsigned short);
      rv = MsgReply( rcvid, rsize, &rep, rsize );
      return;
    case SBC_MREAD:
    case SBC_INTATT:
    case SBC_INTDET:
      ErrorReply(rcvid, ENOTSUP);
      return; // i.e. don't enqueue
    case SBC_QUIT:
      rep.hdr.ret_type = SBRT_NONE;
      rep.hdr.status = SBS_OK;
      rsize = sizeof(subbusd_rep_hdr_t);
      rv = MsgReply( rcvid, rsize, &rep, rsize );
      return;
    default:
      nl_error(4, "Undefined command in incoming_sbreq!" );
  }
  nl_error(4, "No queueing allowed");
}

void init_subbus(dispatch_t *dpp ) {
  /* Setup ionotify pulse handler */
  // init_sbsim(dpp);

}

void shutdown_subbus(void) {
  nl_error( 0, "%d writes, %d reads", n_writes, n_reads);
}
