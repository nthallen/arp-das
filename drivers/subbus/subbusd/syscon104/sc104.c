/* sc104.c
 * subbusd module to interface directly to Syscon/104
 */
#include <stdio.h>
#include <string.h>
#include <sys/neutrino.h>
#include <hw/inout.h>
#include "subbusd_int.h"
#include "nortlib.h"
#include "sc104.h"

#if SC104
int read_ack( unsigned short addr, unsigned short *data ) {
  int i, status;
  out16(SC_SB_LOWB, addr); // Output address
  out8(SC_SB_LOWC, 1); // assert read
  for ( i = WAIT_COUNT; i > 0; i-- ) {
    status = in16(SC_SB_LOWC); // Check read+write bit
    if ( !(status | 0x800) ) break;
  }
  *data = in16(SC_SB_LOWA);
  return((status&0x40) ? i+1 : 0 );
}
#endif

#if SC104
int write_ack(unsigned short addr, unsigned short data) {
  int i, status;
  out16(SC_SB_LOWB, addr); // Output address
  out16(SC_SB_LOWA, data); // Output data
  for ( i = WAIT_COUNT; i > 0; i-- ) {
    status = in16(SC_SB_LOWC);
    if ( !(status & 0x800) ) break;
  }
  return ((status&0x40) ? i+1 : 0 );
}
#endif

/** don't need to serialize access to cmdenbl, since it's I/O port mapped.
 */
void set_cmdenbl(short int val) {
  out16(SC_CMDENBL, val);
}

/**
 * Function to set cmdstrobe value.
 * @param val non-zero value asserts cmdstrobe.
 * @return non-zero no success, zero if operation is not supported.
 *
 *  Function did not exist at all before version 3.10, so
 *  programs intending to use this function should verify that
 *  the resident library version is at least 3.10. The feature
 *  word can also be checked for support, and that is consistent
 *  back to previous versions.
 */
short int set_cmdstrobe(short int value) {
  #if SYSCON
    #if SC104
      out8(SC_SB_LOWC, 8 | (value?0:2));
    #else
      out8(SC_SB_LOWCTRL, 2 | (value?1:0));
    #endif
    return 1;
  #else
    return 0;
  #endif
}

unsigned short read_switches(void) {
  #if READSWITCH
    return in8(SC_SWITCHES) | 0x8000;
  #else
    return 0x8000;
  #endif
}

void set_failure(short int value) {
  #if SIC
    if ( value ) in8(SC_LAMP);
    else out8(SC_LAMP,0);
  #endif
  #if SYSCON
    out8(SC_LAMP,value);
  #endif
}

unsigned char read_failure(void) {
  #if SYSCON
    return in8(SC_LAMP);
  #else
    return 0;
  #endif
}

void tick_sic(void) {
  out8(SC_TICK, 0);
}

void disarm_sic(void) {
  out8(SC_DISARM, 0);
}

/**
 The basic sanity of the incoming message has been
 checked by subbus_io_msg before it gets here,
 so we can at least assume that the message was
 big enough to include the specified message type,
 and that the message type is defined.
 */
void incoming_sbreq( int rcvid, subbusd_req_t *req ) {
  subbus_rep_t rep;
  int rsize, rv;
  
  rep.hdr.status = SBS_OK;
  rep.hdr.ret_type = SBRT_US;

  switch ( req->sbhdr.command ) {
    case SBC_READACK:
      rep.hdr.status =
        read_ack( req->data.d1.data, &rep.data.value ) ?
        SBS_OK : SBS_NOACK;
      break;
    case SBC_WRITEACK:
      rep.hdr.status =
        write_ack( req->data.d0.address, req->data.d0.data ) ?
        SBS_OK : SBS_NOACK;
      rep.hdr.ret_type = SBRT_NONE;
      break;
    case SBC_SETCMDENBL:
      set_cmdenbl(req->data.d1.data);
      rep.hdr.ret_type = SBRT_NONE;
      break;
    case SBC_SETCMDSTRB:
      set_cmdstrobe( req->data.d1.data );
      rep.hdr.ret_type = SBRT_NONE;
      break;
    case SBC_SETFAIL:
      set_failure(req->data.d1.data);
      rep.hdr.ret_type = SBRT_NONE;
      break;
    case SBC_READSW:
      rep.data.value = read_switches();
      break;
    case SBC_READFAIL:
      rep.data.value = read_failure();
      break;
    case SBC_GETCAPS:
      rep.data.capabilities.subfunc = LIBRARY_SUB;
      rep.data.capabilities.features = SUBBUS_FEATURES;
      strncpy(rep.data.capabilities.name, "Subbus Library V4.00: Syscon/104", SUBBUS_NAME_MAX );
      rep.hdr.ret_type = SBRT_CAP;
      break;
    case SBC_TICK:
      tick_sic();
      rep.hdr.ret_type = SBRT_NONE;
      break;
    case SBC_DISARM:
      disarm_sic();
      rep.hdr.ret_type = SBRT_NONE;
      break;
    case SBC_INTATT:
      rv = int_attach(rcvid, req, sreq);
      if (rv != EOK) {
        ErrorReply(rcvid, rv);
        return; // i.e. don't enqueue
      }
      break;
    case SBC_INTDET:
      rv = int_detach(rcvid, req, sreq);
      if (rv != EOK) {
        ErrorReply(rcvid, rv);
        return; // i.e. don't enqueue
      }
      break;
    default:
      nl_error(4, "Undefined command in incoming_sbreq!" );
  }
  rv = MsgReply( rcvid, rsize, &rep, rsize );
}

void init_subbus(dispatch_t *dpp ) {
  int expint_pulse;
  
  if (ThreadCtl(_NTO_TCTL_IO,0) == -1 )
    nl_error( 3, "Error requesting I/O priveleges: %s", strerror(errno) );
  // We won't do mmap_device_io()
  // http://www.qnx.com/developers/articles/article_304_2.html
  out16( SC_SB_RESET, 0 );
  #if !SC104
    #if SYSCON
      out16( SC_SB_LOWCTRL, SC_SB_CONFIG );
    #else
      out8( SC_SB_LOWCTRL, SC_SB_CONFIG & 0xFF );
      out8( SC_SB_HIGHCTRL, (SC_SB_CONFIG >> 8) & 0xFF );
    #endif
  #endif
  expint_pulse =
    pulse_attach(dpp, MSG_FLAG_ALLOC_PULSE, 0, expint_svc, NULL);
  interrupt_init(dpp, expint_pulse );
}

void shutdown_subbus(void) {
  nl_error( 0, "%d writes, %d reads, %d partial reads, %d compound reads",
    n_writes, n_reads, n_part_reads, n_compound_reads );
}
