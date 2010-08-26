/* subbus-usb/src/subbus.c 
 */
#include <sys/neutrino.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "subbus.h"
#include "nortlib.h"
#include "nl_assert.h"
#include "subbusd.h"

#define SUBBUS_VERSION 0x501 /* subbus version 5.01 QNX6 */

static int sb_fd = -1;
static iov_t sb_iov[3];
static subbusd_req_hdr_t sb_req_hdr;
static subbusd_rep_t sb_reply;
static char local_subbus_name[SUBBUS_NAME_MAX];

unsigned short subbus_version = SUBBUS_VERSION;
unsigned short subbus_subfunction; // undefined until initialization
unsigned short subbus_features; // ditto

/**
 @return Number of bytes in reply or -1 on error.
 */
static int send_to_subbusd( unsigned short command, void *data,
		int data_size, unsigned short exp_type ) {
  int rv;
  if ( sb_fd == -1 )
    nl_error( 4, "Attempt to access subbusd before initialization" );
  int n_iov = 1;
  sb_req_hdr.command = command;
  if ( data_size > 0 ) {
    SETIOV( &sb_iov[1], data, data_size );
    ++n_iov;
  }
  rv = MsgSendv( sb_fd, sb_iov, n_iov, &sb_iov[2], 1 );
  if ( rv == -1 )
    nl_error( 3, "Error sending to subbusd: %s",
      strerror(errno) );
  nl_assert( rv >= sizeof(subbusd_rep_hdr_t) );
  if ( sb_reply.hdr.status < 0 ) 
    exp_type = SBRT_NONE;
  nl_assert( sb_reply.hdr.ret_type == exp_type );
  switch ( sb_reply.hdr.ret_type ) {
    case SBRT_NONE:
      nl_assert( rv == sizeof(subbusd_rep_hdr_t));
      break;
    case SBRT_US:
      nl_assert( rv == sizeof(subbusd_rep_hdr_t) + sizeof(unsigned short));
      break;
    case SBRT_CAP:
      nl_assert( rv == sizeof(subbusd_rep_hdr_t) + sizeof(subbusd_cap_t));
      break;
    default:
      nl_error( 4, "Unknown return type: %d", sb_reply.hdr.ret_type );
  }
  return sb_reply.hdr.status;
}


/** Initializes communications with subbusd driver.
    Returns library subfunction on success,
    zero on failure.
 */
int load_subbus(void) {
  int rv;
  if ( sb_fd != -1 ) {
    nl_error( 2, "Attempt to reload subbus" );
    return subbus_subfunction;
  }
  sb_fd = open("/dev/huarp/subbus", O_RDWR );
  if ( sb_fd == -1 ) {
    nl_error( -2, "Error opening subbusd: %s", strerror(errno));
    return 0;
  }
  SETIOV( &sb_iov[0], &sb_req_hdr, sizeof(sb_req_hdr) );
  sb_req_hdr.iohdr.type = _IO_MSG;
  sb_req_hdr.iohdr.combine_len = 0;
  sb_req_hdr.iohdr.mgrid = SUBBUSD_MGRID;
  sb_req_hdr.iohdr.subtype = 0;
  sb_req_hdr.sb_kw = SB_KW;
  SETIOV( &sb_iov[2], &sb_reply, sizeof(sb_reply) );
  rv = send_to_subbusd( SBC_GETCAPS, NULL, 0, SBRT_CAP );
  if ( rv != SBS_OK )
    nl_error( 4, "Expected SBS_OK while getting capabilities" );
  subbus_subfunction = sb_reply.data.capabilities.subfunc;
  subbus_features = sb_reply.data.capabilities.features;
  strncpy(local_subbus_name, sb_reply.data.capabilities.name, SUBBUS_NAME_MAX);
  local_subbus_name[SUBBUS_NAME_MAX-1] = '\0'; // guarantee nul-term.
  return subbus_subfunction;
}

/**
 * Returns the hardware name string as originally retrieved from
 * subbusd during load_subbus().
 */
char *get_subbus_name(void) {
  if ( sb_fd == -1 )
    nl_error( 4, "Attempt to read subbus_name before initialization" );
  return( local_subbus_name );
}

/**
 @return non-zero if hardware read acknowledge was observed.
 */
int read_ack( unsigned short addr, unsigned short *data ) {
  int rv, rc;
  subbusd_req_data1 rdata;

  rdata.data = addr;
  rv = send_to_subbusd( SBC_READACK, &rdata, sizeof(rdata), SBRT_US );
  *data = sb_reply.data.value;
  switch ( rv ) {
    case SBS_ACK: rc = 1;
    case SBS_NOACK: rc = 0;
    default:
      nl_error( 4, "Invalid status response to read_ack(): %d",
	rv );
  }
  return rc;
}

unsigned short read_subbus(unsigned short addr) {
  unsigned short data;
  read_ack(addr, &data);
  return data;
}

unsigned short sbrb(unsigned short addr) {
  unsigned int word;
  
  word = read_subbus(addr);
  if (addr & 1) word >>= 8;
  return(word & 0xFF);
}

/* returns zero if no acknowledge */
unsigned short sbrba(unsigned short addr) {
  unsigned short word;
  
  if ( read_ack( addr, &word ) ) {
    if (addr & 1) word >>= 8;
    return( word & 0xFF );
  } else return 0;
}

/* returns zero if no acknowledge */
unsigned int sbrwa(unsigned short addr) {
  unsigned short word;
  
  if ( read_ack( addr, &word ) )
    return word;
  else return 0;
}

/**
 @return non-zero value if the hardware acknowledge is
 observed. Historically, the value recorded the number
 of iterations in the software loop waiting for
 the microsecond timeout.
 */
int write_ack(unsigned short addr, unsigned short data) {
  int rv, rc;
  subbusd_req_data0 wdata;

  wdata.address = addr;
  wdata.data = data;
  rv = send_to_subbusd( SBC_WRITEACK, &wdata, sizeof(wdata), SBRT_NONE );
  switch (rv ) {
    case SBS_ACK: rc = 1;
    case SBS_NOACK: rc = 0;
    default:
      nl_error( 4, "Invalid status response to read_ack(): %d",
	rv );
  }
  return rc;
}

/** This is an internal function for sending messages
 * with a single unsigned short argument and a simple
 * status return.
 @return non-zero on success. Zero if unsupported.
 */
static int set_CSF( unsigned short command, unsigned short val ) {
  int rv;
  subbusd_req_data1 csf_data;

  switch ( command ) {
    case SBC_SETCMDENBL:
    case SBC_SETCMDSTRB:
    case SBC_SETFAIL:
      break;
    default:
      nl_error( 4, "Invalid command in set_CSF: %d", command );
  }
  csf_data.data = val;
  rv = send_to_subbusd( command, &csf_data, sizeof(csf_data), SBRT_NONE );
  return( (rv == SBS_OK) ? 1 : 0 );
}


/** Set cmdenbl value.
 @return non-zero on success. Zero if not supported.
 */
int set_cmdenbl(int val) {
  return send_CSF( SBC_SETCMDENBL, val ? 1 : 0);
}

/**
  Function did not exist at all before version 3.10, so
  programs intending to use this function should verify that
  the resident library version is at least 3.10. The feature
  word can also be checked for support, and that is consistent
  back to previous versions.
 @param value 1 turns on cmdstrobe, 0 turns off cmdstrobe
 @return non-zero on success, zero if operation isn't supported.
 */
int set_cmdstrobe(int val) {
  return send_CSF(SBC_SETCMDSTRB, val ? 1 : 0);
}

/**
 Sets the value of a dedicated set of indicator lights, usually
 located on a control panel on the instrument and/or in the
 cockpit of the aircraft. For each bit of the input argument,
 a non-zero value indicates the associated light should be on.
 
 By convention, the least significant bit is associated with the
 main "fail light" located in the cockpit on aircraft instruments,
 indicating that the instrument is not acquiring data..
 This light (and the associated bit value on readback) will also
 be set by the system controller's two minute timeout circuit.
 
 @see read_failure()
 @param value Binary-encoded light settings.
 */
int set_failure(unsigned short value) {
  return send_CSF( SBC_SETFAIL, value );
}

/** Internal function to handle read_switches() and read_failure(),
  which take no arguments, return unsigned short or zero if
  the function is not supported.
  */
static unsigned short read_special( unsigned short command ) {
  int rv;
  rv = send_to_subbusd( command, NULL, 0, SBRT_US );
  return (rv == SBS_OK) ? sb_reply.data.value : 0;
}

/**
 Reads the positions of a dedicated set of system mode switches,
 usually located on a control panel on the instrument.
 @return The binary-encoded switch positions, or zero on error.
 */
unsigned short read_switches(void) {
  return read_special( SBC_READSW );
}

/**
 The value reported represents the current state of the indicator
 lights. As noted in read_switches(), the least significant bit
 is associated the the main "fail light" located in the cockpit.
 This light can be lit via set_failure() or the system controller's
 two minute timeout circuit. In either case, read_failure() will
 report the actual state of the light.
 @return The binary-encoded value of the indicator light settings.
 */
unsigned short read_failure(void) {
  return read_special( SBC_READFAIL );
}

/**
 Historically, tick_sic() has been associated with two timers.
 The first is a 2-second timeout that can reboot the system.
 The second is a 2-minute timeout that lights the main fail
 light indicating that the instrument is not acquiring data.

 It is unclear whether the new syscon_usb will support the
 reboot timer or rely on a motherboard-specific watchdog
 timer.
 */
int tick_sic( void ) {
  return send_to_subbusd( SBC_TICK, NULL, 0, SBRT_NONE );
}

/**
 If system controller is associated with a watchdog timer
 that can reboot the system, this command disables that
 timer.
 */
int disarm_sic(void) {
  return send_to_subbusd( SBC_DISARM, NULL, 0, SBRT_NONE );
}

int subbus_int_attach( char *cardID, unsigned short address,
      unsigned short region, struct sigevent *event ) {
  subbusd_req_data2 idata;
  nl_assert(cardID != NULL);
  strncpy( idata.cardID, cardID, 8);//possibly not nul-terminated
  idata.address = address;
  idata.region = region;
  idata.event = *event;
  return send_to_subbusd( SBC_INTATT, &idata, sizeof(idata), SBRT_NONE );
}

int subbus_int_detach( char *cardID ) {
  subbusd_req_data3 idata;
  nl_assert(cardID != NULL);
  strncpy( idata.cardID, cardID, 8);//possibly not non-terminated
  return send_to_subbusd( SBC_INTDET, &idata, sizeof(idata), SBRT_NONE );
}
