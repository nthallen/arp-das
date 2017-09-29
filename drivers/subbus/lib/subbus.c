/* subbus-usb/src/subbus.c 
 */
#include <sys/neutrino.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
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
 @return Status reply from subbusd. Terminates if
 communication with subbusd fails.
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
  if ( sb_reply.hdr.ret_type != exp_type ) {
    nl_error( 4, "Return type for command %u should be %d, is %d",
      command, exp_type, sb_reply.hdr.ret_type );
  }
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
    case SBRT_MREAD:
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
    nl_error( -2, "Attempt to reload subbus" );
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
    case SBS_ACK: rc = 1; break;
    case -ETIMEDOUT:
    case SBS_NOACK: rc = 0; break;
    default:
      nl_error( 4, "Invalid status response to read_ack(): %d",
	rv );
  }
  return rc;
}

/**
 @return Cached read value or zero if address is invalid.
 */
unsigned short cache_read( unsigned short addr ) {
  int rv;
  subbusd_req_data1 rdata;
  unsigned short data;

  rdata.data = addr;
  rv = send_to_subbusd( SBC_READCACHE, &rdata, sizeof(rdata), SBRT_US );
  data = sb_reply.data.value;
  switch ( rv ) {
    case SBS_ACK: break;
    case -ETIMEDOUT:
    case SBS_NOACK: data = 0; break;
    default:
      nl_error( 4, "Invalid status response to cache_read(): %d",
	rv );
  }
  return data;
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
    case SBS_ACK: rc = 1; break;
    case -ETIMEDOUT:
    case SBS_NOACK: rc = 0; break;
    default:
      nl_error( 4, "Invalid status response to write_ack(): %d",
	rv );
  }
  return rc;
}

/**
 @return non-zero value if the hardware acknowledge is
 observed. Historically, the value recorded the number
 of iterations in the software loop waiting for
 the microsecond timeout.
 */
int cache_write(unsigned short addr, unsigned short data) {
  int rv, rc;
  subbusd_req_data0 wdata;

  wdata.address = addr;
  wdata.data = data;
  rv = send_to_subbusd( SBC_WRITECACHE, &wdata, sizeof(wdata), SBRT_NONE );
  switch (rv ) {
    case SBS_ACK: rc = 1; break;
    case -ETIMEDOUT:
    case SBS_NOACK: rc = 0; break;
    default:
      nl_error( 4, "Invalid status response to cache_write(): %d",
	rv );
  }
  return rc;
}

/** This is an internal function for sending messages
 * with a single unsigned short argument and a simple
 * status return.
 @return non-zero on success. Zero if unsupported.
 */
static int send_CSF( unsigned short command, unsigned short val ) {
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
  return send_to_subbusd( SBC_INTATT, &idata, sizeof(idata), SBRT_US );
}

int subbus_int_detach( char *cardID ) {
  subbusd_req_data3 idata;
  nl_assert(cardID != NULL);
  strncpy( idata.cardID, cardID, 8);//possibly not non-terminated
  return send_to_subbusd( SBC_INTDET, &idata, sizeof(idata), SBRT_US );
}

/**
 * Requests subbusd to terminate. subbusd will wait until
 * all connections are closed.
 * @return SBS_OK on success.
 */
int subbus_quit(void) {
  if ( sb_fd == -1 ) return 0;
  return send_to_subbusd( SBC_QUIT, NULL, 0, SBRT_NONE );
}

/**
 * @param req structure defining multi-read request
 * @param data pointer to where read data should be stored
 * Passes the raw command directly to the subbus driver and parses
 * the return string for a multi-read. Up to req->n_reads values will be
 * written into the array pointed to by the data argument.
 * If the request returns less data than requested, a warning will
 * be issued unless suppressed by set_response().
 * @return Zero on success. If return value is negative, it is the
 * error code returned by the subbusd driver and no values are reported.
 * If it is positive (SBS_NOACK), it indicates that although the
 * requested number of values are reported, at least one of the
 * values did not have an acknowledge, and a zero value was reported.
 */
int mread_subbus( subbus_mread_req *req, unsigned short *data) {
  // rv should be the number of bytes retuned from subbusd into sb_reply.
  int nw, rv;
  if ( req == NULL ) return 200;
  rv = mread_subbus_nw(req, data, &nw);
  if (nw < req->n_reads && nl_response > 0)
    nl_error(MSG_WARN, "mread returned %d/%d words", nw, req->n_reads);
  return rv;
}

/**
 * @param req structure defining multi-read request
 * @param data pointer to where read data should be stored
 * @param nwords pointer to where actual read length should be stored
 * Passes the raw command directly to the subbus driver and parses
 * the return string for a multi-read. Up to req->n_reads values will be
 * written into the array pointed to by the data argument.
 * If the request returns more data than requested, and error
 * will be reported (MSG_ERROR).
 * @return Zero on success. If return value is negative, it is the
 * error code returned by the subbusd driver and no values are reported.
 * If it is positive (SBS_NOACK), it indicates that although the
 * requested number of values are reported, at least one of the
 * values did not have an acknowledge, and a zero value was reported.
 */
int mread_subbus_nw(subbus_mread_req *req, unsigned short *data, unsigned short *nwords) {
  int rv;
  int nw = 0;
  if ( req == NULL ) return 200;
  rv = send_to_subbusd( SBC_MREAD, req, req->req_len, SBRT_MREAD );
  if ( rv >= 0 ) {
    int i;
    nw = sb_reply.data.mread.n_reads;
    if (nw > req->n_reads) {
      nl_error(MSG_ERROR, "mread expected %d words, returned %d",
        req->n_reads, nw);
      nw = req->n_reads;
    }
    for ( i = 0; i < nw; ++i ) {
      data[i] = sb_reply.data.mread.rvals[i];
    }
  }
  if (nwords != 0)
    *nwords = nw;
  return rv;
}

/**
 Packages a request string into a newly allocated structure that
 can be passed to mread_subbus(). Called by pack_mread_request()
 and pack_mread_requests(). The req_str syntax is:

 \code{.unparsed}
    <req>
      : M <count> '#' <addr_list> '\n'
    <addr_list>
      : <addr_list_elt>
      : <addr_list> ',' <addr_list_elt>
    <addr_list_elt>
      : <addr>
      : <addr> ':' <incr> ':' <addr>
      : <count> '@' <addr>
      : <addr> '|' <count> '@' <addr>

    <count>, <addr>, <incr> are all 1-4 hex digits
  \endcode
  
  The four current addr_list_elt syntaxes have the following
  meaning:

  \code{.unparsed}  
  <addr>:
    Read one word from the specified address
  <addr1> ':' <incr> ':' <addr2>:
    Read one word from each address, starting at <addr1>,
    incremented the address by <incr> and ending
    when the address exceeds <addr2>
  <count> '@' <addr>:
    Read <count> words from <addr>
  <addr1> '|' <count> '@' <addr>:
    Read count1 from <addr1> and read count1 or <count> words
    from <addr>, whichever is less. Returns between 1 and
    <count>+1 words.
  \endcode

 @return the newly allocated request structure.
 */
static subbus_mread_req *pack_mread( int req_len, int n_reads, const char *req_str ) {
  int req_size = 2*sizeof(unsigned short) + req_len + 1;
  subbus_mread_req *req = (subbus_mread_req *)new_memory(req_size);
  req->req_len = req_size;
  req->n_reads = n_reads;
  strcpy( req->multread_cmd, req_str );
  return req;
}

/**
 * Takes a zero-terminated list of addresses, generates the appropriate
 * text request string and invokes pack_mread().
 * @return the newly allocated request structure.
 */
subbus_mread_req *pack_mread_requests( unsigned int addr, ... ) {
  unsigned short addrs[50];
  int n_reads = 0;
  
  { unsigned int val = addr;
    va_list va;
    if ( addr == 0 ) return NULL;
    va_start( va, addr );
    while ( val != 0 && n_reads < 50 ) {
      addrs[n_reads++] = (unsigned short) val;
      val =  va_arg(va, unsigned int);
    }
    va_end(va);
  }
  { char buf[256];
    int nc = 0;
    int space = 256;
    int i = 0;
    int nb;

    nb = snprintf( buf, space, "M%X#", n_reads );
    nl_assert(nb < space);
    nc += nb;
    space -= nb;
    while ( i < n_reads ) {
      nb = 0;
      if (i+2 < n_reads &&
          addrs[i] <= addrs[i+1] &&
          addrs[i+1] <= addrs[i+2] ) {
        unsigned d1 = addrs[i+1]-addrs[i];
        unsigned d2 = addrs[i+2]-addrs[i+1];
        if ( d1 == d2 ) {
          // We'll use either the s:i:e syntax or the n@a syntax.
          int j;
          for ( j = 2; i+j+1 < n_reads; ++j) {
            if ( addrs[i+j] + d1 != addrs[i+j+1] )
              break;
          }
          // Now we'll handle samples from i to i+j
          if ( d1 == 0 ) {
            nb = snprintf( buf+nc, space, "%X@%X,", j-i+1, addrs[i] );
          } else {
            nb = snprintf( buf+nc, space, "%X:%X:%X,",
              addrs[i], d1, addrs[i+j] );
          }
          i += j+1;
        }
      }
      if (nb == 0) {
        // We did not use an optimization, so just output an address
        nb = snprintf( buf+nc, space, "%X,", addrs[i++] );
      }
      if ( nb >= space ) {
        nl_error( 2, "Buffer overflow in pack_mread_requests()" );
        return NULL;
      }
      nc += nb;
      space -= nb;
    }
    // replace the trailing comma with a newline:
    buf[nc-1] = '\n';
    return pack_mread( nc, n_reads, buf );
  }
}

/**
 * Takes a multi-read <addr-list> string and invokes pack_mread().
 * @return the newly allocated request structure.
 */
subbus_mread_req *pack_mread_request( int n_reads, const char *req ) {
  char buf[256];
  int space = 256;
  int nb;

  nb = snprintf( buf, space, "M%X#%s\n", n_reads, req );
  if ( nb >= space ) {
    nl_error( 2, "Buffer overflow in pack_mread_request()" );
    return NULL;
  }
  return pack_mread( nb, n_reads, buf );
}
