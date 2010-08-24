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

#define LIBRARY_SUB SB_SUBBUSD
#define SUBBUS_VERSION 0x501 /* subbus version 5.01 QNX6 */

static int sb_fd = -1;
static iov_t sb_iov[3];
static struct _io_msg sb_hdr;
static char sb_buf[SUBBUSD_MAX_REQUEST];

int load_subbus(void) {
  sb_fd = open("/dev/huarp/subbus", O_RDWR );
  if ( sb_fd == -1 )
    nl_error( 3, "Error opening subbusd: %s", strerror(errno));
  SETIOV( &sb_iov[0], &sb_hdr, sizeof(sb_hdr) );
  sb_hdr.type = _IO_MSG;
  sb_hdr.combine_len = 0;
  sb_hdr.mgrid = SUBBUSD_MGRID;
  sb_hdr.subtype = 0;
  SETIOV( &sb_iov[2], sb_buf, SUBBUSD_MAX_REQUEST );
  return LIBRARY_SUB;
}

unsigned int subbus_version = SUBBUS_VERSION;
unsigned int subbus_features = SUBBUS_FEATURES;
unsigned int subbus_subfunction = LIBRARY_SUB;


#define SB_BASE_NAME "Subbus Library V5.01"
/**
 * Should be expanded to query subbusd.
 */
char *get_subbus_name(void) {
  return SB_BASE_NAME;
}

static int send_to_subbusd( char *buf, int nb ) {
  int rv;
  if ( sb_fd != -1 ) {
    SETIOV( &sb_iov[1], buf, nb );
    rv = MsgSendv( sb_fd, sb_iov, 2, &sb_iov[2], 1 );
  } else {
    errno = EBADF;
    rv = -1;
  }
  return rv;
}


int read_ack( unsigned short addr, unsigned short *data ) {
  int n_out, n_in;
  char buf[SUBBUSD_MAX_REQUEST];

  sprintf( buf, "R%04X\n", addr & 0xFFFF );
  n_in = send_to_subbusd( buf, 7 );
  if ( n_in < 1 )
    nl_error( 3, "Error reading from subbusd: %s", strerror(errno) );
  else if (n_in != 6 ) {
    if ( sb_buf[0] == 'E' )
      nl_error( 1, "Error %c from syscon in read_ack", buf[1] ); 
    else nl_error( 3,
	"Unexpected input count in read_ack: %d (expected 6)", n_in );
  } else {
    int i, nv;
    unsigned short idata = 0;
    for ( i = 1; i <= 4; i++ ) {
      int c = sb_buf[i];
      if ( isdigit(c) ) nv = c - '0';
      else if (isxdigit(c)) {
        if (isupper(c)) nv = c - 'A' + 10;
        else nv = c - 'a' + 10;
      } else {
        nl_error( 1, "Invalid character in read_ack" );
        nv = 0;
      }
      idata = (idata<<4) + nv;
    }
    *data = idata;
  }
  return((sb_buf[0] == 'R') ? 1 : 0 );
}

unsigned short read_subbus(unsigned short addr) {
  unsigned short data;
  read_ack(addr, &data);
  return data;
}

unsigned int sbb(unsigned short addr) {
  unsigned int word;
  
  word = read_subbus(addr);
  if (addr & 1) word >>= 8;
  return(word & 0xFF);
}

/* returns zero if no acknowledge */
unsigned int sbba(unsigned short addr) {
  unsigned short word;
  
  if ( read_ack( addr, &word ) ) {
  	if (addr & 1) word >>= 8;
  	return( word & 0xFF );
  } else return 0;
}

/* returns zero if no acknowledge */
unsigned int sbwa(unsigned short addr) {
  unsigned short word;
  
  if ( read_ack( addr, &word ) )
    return word;
  else return 0;
}

int write_ack(unsigned short addr, unsigned short data) {
  int n_out, n_in;
  char buf[12];

  sprintf( buf, "W%04X:%04X\n", addr, data );
  n_in = send_to_subbusd( buf, 11 );
  switch (n_in) {
    case -1:
      nl_error( 3, "Error writing to subbusd in write_ack: %s",
	      strerror(errno) );
    case 0:
      nl_error( 3, "Empty response from subbusd in write_ack" );
    case 2:
      switch ( sb_buf[0] ) {
	case 'w': return 0;
	case 'W': return 1;
	case 'E':
	  nl_error( 1, "Error '%c' from subbusd in write_ack", sb_buf[1] );
	  return 0;
	default:
	  nl_error( 3, "Unexpected response '%c' in write_ack", sb_buf[0] );
      }
      break;
    case 1:
    default:
      nl_error( 3, "Unexpected response '%c' length %d in write_ack",
	sb_buf[0], n_in );
  }
  return 0;
}

static void send_CS( char code, int val ) {
  int n_in;
  char buf[12];

  nl_assert( code == 'S' || code == 'C' );
  sprintf( buf, "%c%d\n", code, val ? 1 : 0 );
  
  n_in = send_to_subbusd( buf, 3 );
  switch (n_in) {
    case -1:
      nl_error( 3, "Error writing to subbusd in send_CS(): %s",
	      strerror(errno) );
    case 0:
      nl_error( 3, "Empty response from subbusd in send_CS()" );
    case 1:
      nl_error( 3, "Unexpected response '%c' in send_CS()", sb_buf[0] );
    case 3:
      if ( sb_buf[0] == code ) {
	if ( sb_buf[1] - '0' != (val ? 1 : 0)) {
	  nl_error( 1, "send_CS() returned wrong value" );
	}
	break;
      } else if (sb_buf[0] == 'E') {
	nl_error( 1, "Error '%c' from subbusd in send_CS()", sb_buf[1] );
	break;
      }
    case 2:
    default:
      nl_error( 3, "Unexpected response '%c' length %d in send_CS()",
	sb_buf[0], n_in );
  }
}

/* CMDENBL "Cn\n" where n = 0 or 1. Response should be the same */
void set_cmdenbl(int val) {
  send_CS( 'C', val );
}

/**
 Reads the positions of a dedicated set of system mode switches,
 usually located on a control panel on the instrument.
 @return The binary-encoded switch positions, or zero on error.
 */
unsigned int read_switches(void) {
  // Send 'D' command
  return 0;
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
void set_failure(int value) {
  #if SET_FAIL
    #if SIC
      if ( value ) in8(SC_LAMP);
      else out8(SC_LAMP,0);
    #endif
    #if SYSCON
      out8(SC_LAMP,value);
    #endif
  #endif
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
unsigned char read_failure(void) {
  #if SET_FAIL && SYSCON
    return in8(SC_LAMP);
  #else
    return 0;
  #endif
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
short int set_cmdstrobe(short int value) {
  send_CS('S', value);
  return 1;
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
int  tick_sic(void) {
  return 0;
}

/**
 If system controller is associated with a watchdog timer
 that can reboot the system, this command disables that
 timer.
 */
void disarm_sic(void) {
}
