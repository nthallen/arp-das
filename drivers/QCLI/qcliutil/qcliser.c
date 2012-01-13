// #include <sys/dev.h> // need for dev_read() in QNX4
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "nortlib.h"
#include "oui.h"
#include "qcliutil.h"

static int qcli_fd;

/* unsigned short read_qcli(int fresh);
   If fresh is non-zero, it means a command has just
   been issued, and we want to give the QCLI time
   to process that command before requesting status.
   I believe that with the serial interface, we
   will need to send a NOP command every time we
   want to read from the QCLI. My reasoning is
   that when fresh is non-zero, although a status
   word is already being transmitted, we will
   need to request another 'fresher' one, and
   when fresh is zero, no prior command has
   been issued, so we need to request a new
   status anyway. The only difference between
   fresh and !fresh is whether we add an
   additional delay before sending the NOP
*/
#define MAX_STATUS_WORDS 1024
#define SBUFSIZE (MAX_STATUS_WORDS*sizeof(unsigned short))
unsigned short read_qcli( int fresh ) {
  unsigned char buf[SBUFSIZE];
  unsigned short status;
  int rv;
  static int responding = 1;
  
  /* I'll remove this delay since the write always
     has a delay */
  if ( fresh ) delay3msec();
  while ( 1 ) {
    write_qcli( QCLI_NOOP );
    rv = readcond( qcli_fd, buf, SBUFSIZE, SBUFSIZE, 1, 4 );
    if ( rv == 0 || (rv == -1 && errno == EAGAIN) ) {
      if ( responding ) {
        nl_error( 2, "QCLI is not responding" );
        responding = 0;
      }
      return 0;
    } else if ( rv == -1 ) {
      nl_error( 1, "readcond(qcli_fd) returned error %d: %s",
            errno, strerror(errno) );
      return 0;
    } else if ( rv & 1 ) {
      nl_error( 2, "QCLI returned an odd number of bytes (%d)", rv );
      continue;
    } else {
      if ( rv == SBUFSIZE )
        nl_error( 1, "Input buffer is full" );
      if ( !responding )
        nl_error( 0, "QCLI is responding" );
      responding = 1;
      status = (buf[rv-2]<<8) + buf[rv-1];
      return status;
    }
  }
}

void write_qcli( unsigned short value ) {
  char buf[3];
  int rv;
  
  buf[0] = value >> 8;
  buf[1] = value & 0xFF;
  rv = write( qcli_fd, buf, 2 );
  if ( rv == -1 )
    nl_error( 1, "write(qcli_fd,2) returned error %d: %s",
      errno, strerror(errno) );
  else if ( rv != 2 )
    nl_error( 1, "write(qcli_fd,2) returned %d", rv );
  /* This delay is intended to prevent the FIFO from
     overflowing */
  /* delay3msec(); */
}

unsigned short wr_rd_qcli( unsigned short value ) {
  write_qcli( value );
  return read_qcli(1);
}

void wr_stop_qcli(  unsigned short value ) {
  wr_rd_qcli( value );
}

void qcli_port_init( char *port ) {
  qcli_fd = open( port, O_RDWR );
  if ( qcli_fd == -1 )
	nl_error( 3, "Unable to open serial port '%s': %s",
	  port, strerror(errno) );
}

void qclisrvr_init( int argc, char **argv ) {
  int c;
  char *port = "/dev/ser1";

  optind = OPTIND_RESET; /* start from the beginning */
  opterr = 0; /* disable default error message */
  while ((c = getopt(argc, argv, opt_string)) != -1) {
	switch (c) {
	  case 'd':
		port = optarg;
		break;
	  case '?':
		nl_error(3, "Unrecognized Option -%c", optopt);
	}
  }
  qcli_port_init( port );
}
