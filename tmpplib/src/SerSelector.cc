#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <termios.h>
#include "SerSelector.h"
#include "nortlib.h"
#include "nl_assert.h"
#include "msg.h"

TM_Selectee::TM_Selectee( const char *name, void *data,
      unsigned short size ) : Selectee() {
  TMid = Col_send_init( name, data, size, 0 );
  if ( TMid ) {
    fd = TMid->fd;
    flags = Selector::Sel_Write;
  }
}

/**
 * Issues Col_send_reset()
 */
TM_Selectee::~TM_Selectee() {
  Col_send_reset(TMid);
  fd = -1;
}

/**
 * Calls Col_send() and sets gflag(0)
 */
int TM_Selectee::ProcessData(int flag) {
  Col_send(TMid);
  Stor->set_gflag(0);
  return 0;
}

/**
 * Opens the specified channel using tm_dev_name() and
 * tm_open_name(). By default, tm_open_name will produce a
 * fatal error if the resulting path is not found.
 */
Cmd_Selectee::Cmd_Selectee( const char *name ) : Selectee() {
  fd = tm_open_name( tm_dev_name(name), NULL, O_RDONLY );
  flags = Selector::Sel_Read;
}

int Cmd_Selectee::ProcessData(int flag) {
  return 1;
}

/**
 * @param path The full path to the serial device
 * @param open_flags Flags from <fcntl.h> passed to open()
 * @param bufsz The size buffer to be allocated.
 */
Ser_Sel::Ser_Sel(const char *path, int open_flags, int bufsz )
    : Selectee() {
  fd = tm_open_name(path, NULL, open_flags);
  switch (open_flags & O_ACCMODE) {
    case O_RDWR:
    case O_RDONLY:
      flags |= Selector::Sel_Read;
      break;
  }
  buf = (char *)new_memory(bufsz);
  bufsize = bufsz;
  nc = cp = 0;
  n_fills = n_empties = 0;
  total_errors = 0;
  total_suppressed = 0;
  n_errors = 0;
  n_suppressed = 0;
}

/**
 * Frees the allocated buffer and reports statistics.
 */
Ser_Sel::~Ser_Sel() {
  free_memory(buf);
  buf = 0;
  nl_error( 0, "n_fills: %d  n_empties: %d "
    "total_errors: %d total_suppressed: %d",
    n_fills, n_empties, total_errors, total_suppressed );
}

/**
 * Initializes the serial parameters for the device. The min
 * and time parameters can be used to optimize reads. See
 * tcsetattr VMIN and VTIME parameters for more information.
 * @param baud The desired baud rate
 * @param bits number of data bits (5-8)
 * @param par 'n', 'e', 'o', 'm', 's' for none, even, odd, mark or space.
 * @param stopbits The number of stop bits: 1 or 2
 * @param min The minimum number of characters to respond to
 * @param time The time gap value
 */
void Ser_Sel::setup( int baud, int bits, char par, int stopbits,
                int min, int time ) {
  struct termios termios_p;
  int bitsflag;

  if ( fd < 0 ) return;
  if ( tcgetattr( fd, &termios_p) ) {
    nl_error( 2, "Error on tcgetattr: %s", strerror(errno) );
    return;
  }
  termios_p.c_iflag = 0;
  termios_p.c_lflag &= ~(ECHO|ICANON|ISIG|ECHOE|ECHOK|ECHONL);
  termios_p.c_cflag = CLOCAL|CREAD;
  termios_p.c_oflag &= ~(OPOST);
  termios_p.c_ispeed = termios_p.c_ospeed = baud;
  switch (bits) {
    case 5: bitsflag = CS5; break;
    case 6: bitsflag = CS6; break;
    case 7: bitsflag = CS7; break;
    case 8: bitsflag = CS8; break;
    default:
      nl_error( 3, "Invalid bits value: %d", bits );
  }
  termios_p.c_cflag |= bitsflag;
  switch (par) {
    case 'n': bitsflag = 0; break;
    case 'e': bitsflag = PARENB; break;
    case 'o': bitsflag = PARENB|PARODD; break;
    case 'm': bitsflag = PARENB|PARODD|PARSTK; break;
    case 's': bitsflag = PARENB|PARSTK; break;
    default:
      nl_error( 3, "Invalid parity selector: '%c'", par );
  }
  termios_p.c_cflag |= bitsflag;
  switch (stopbits) {
    case 1: break;
    case 2: termios_p.c_cflag |= CSTOPB; break;
    default:
      nl_error(3, "Invalid number of stop bits: %d", stopbits );
  }
  termios_p.c_cc[VMIN] = min;
  termios_p.c_cc[VTIME] = time;
  if ( tcsetattr(fd, TCSANOW, &termios_p) )
    nl_error( 2, "Error on tcsetattr: %s", strerror(errno) );
}

/**
 * Reads characters from the device, reporting any errors.
 * Guarantees that buf is NUL-terminated, and sets nc to the
 * total number of characters. Each call to fillbuf() increments
 * the n_fills counter, which is reported at termination
 * @return non-zero on error.
 */
int Ser_Sel::fillbuf() {
  int i;
  ++n_fills;
  i = read( fd, &buf[nc], bufsize - 1 - nc );
  if ( i < 0 ) {
    if ( errno == EAGAIN || errno == EINTR )
      return 0;
    nl_error( 2, "Error %d on read from serial port", errno );
    return 1;
  }
  nc += i;
  buf[nc] = '\0';
  return 0;
}

/**
 * Each call to consume() increments the n_empties counter which is
 * reported at termination. If n_fills is much greater than n_empties,
 * you may need to adjust your min and time settings for more efficient
 * operation.
 * @param nchars number of characters to remove from front of buffer
 */
void Ser_Sel::consume(int nchars) {
  if ( nchars > 0 ) {
    ++n_empties;
    if ( nchars < nc ) {
      int nb = nc - nchars;
      memmove(&buf[0], &buf[nchars], nb+1);
      nc = nb;
    } else {
      nc = 0;
    }
    cp = 0;
  }
}

#define QERR_THRESHOLD 5
/**
 * Reports the error message, provided the qualified error count
 * is not above the limit. report_ok() will decrement the qualified
 * error count. It is assumed that all messages are of severity
 * MSG_ERR. Messages at other levels, either more or less severe,
 * should be sent directly to msg().
 */
void Ser_Sel::report_err( const char *fmt, ... ) {
  ++total_errors;
  if ( n_errors < QERR_THRESHOLD )
    ++n_errors;
  if ( n_suppressed == 0 && n_errors < QERR_THRESHOLD ) {
    va_list args;

    va_start(args, fmt);
    msgv( 2, fmt, args );
    va_end(args);
    if (nc)
      msg( 2, "Input was: '%s'", ascii_escape(buf) );
  } else {
    if ( !n_suppressed )
      msg( 2, "Error threshold reached: suppressing errors" );
    ++n_suppressed;
    ++total_suppressed;
  }
}

/**
 * Indicate that data has successfully been received.
 */
void Ser_Sel::report_ok() {
  if ( n_errors > 0 ) {
    if ( --n_errors <= 0 && n_suppressed ) {
      msg( 0, "Error recovery: %d error messages suppressed", n_suppressed );
      n_suppressed = 0;
    }
  }
}

/**
 * Parsing utility function that searches forward in the buffer for the
 * specified start character. Updates cp to point just past the start
 * char. If the character is not found, the buffer is emptied.
 * @param c The search character
 * @return zero if the character is found.
 */
int Ser_Sel::not_found( char c ) {
  while ( cp < nc ) {
    if ( buf[cp++] == c )
      return 0;
  }
  if ( nc ) {
    report_err( "Synch char '%c' not found", c );
    nc = cp = 0;
  }
  return 1;
}

/**
 * Parsing utility function to read in a decimal integer starting
 * at the current position.
 * @param[out] val The integer value
 * @return zero if an integer was converted, non-zero if the current char is not a digit.
 */
int Ser_Sel::not_int( int &val ) {
  if ( isdigit(buf[cp]) ) {
    val = buf[cp++] - '0';
    while ( isdigit(buf[cp]) ) {
      val = 10*val + buf[cp++] - '0';
    }
    return 0;
  } else {
    if ( cp < nc )
      report_err( "Expected int at column %d", cp );
    return 1;
  }
}

/**
 * Parsing utility function to check that the string matches the
 * input at the current position. On success, advances cp to just
 * after the matched string. On failure, cp points to the first
 * character that does not match. If only a partial record was
 * received, that could be the NUL at the end of the buffer.
 * @param str The comparison string.
 * @return zero if the string matches the input buffer.
 */
int Ser_Sel::not_str( const char *str ) {
  int start_cp = cp;
  const char *s = str;
  if ( cp < 0 || cp > nc || nc < 0 || nc >= bufsize || buf == 0 )
    msg( 4, "Ser_Sel precondition failed: "
      "cp = %d, nc = %d, bufsize = %d, buf %s",
      cp, nc, bufsize, buf ? "not NULL" : "is NULL" );
  while ( *s != '\0' ) {
    if ( *s != buf[cp] ) {
      if ( cp < nc )
        report_err( "Expected string '%s' at column %d",
          str, start_cp );
      return 1;
    }
    ++s;
    ++cp;
  }
  return 0;
}

/**
 * Parsing utility function to convert a string in the input
 * buffer to a float value. Updates cp to point just after the
 * converted string on success.
 * @param val[out] The converted value
 * @return zero if the conversion succeeded.
 */
int Ser_Sel::not_float( float &val ) {
  char *endptr;
  int ncf;
  if ( cp < 0 || cp > nc || nc < 0 || nc >= bufsize || buf == 0 )
    msg( 4, "Ser_Sel precondition failed: "
      "cp = %d, nc = %d, bufsize = %d, buf %s",
      cp, nc, bufsize, buf ? "not NULL" : "is NULL" );
  val = strtof( &buf[cp], &endptr );
  ncf = endptr - &buf[cp];
  if ( ncf == 0 ) {
    if ( cp < nc )
      report_err( "Expected float at column %d", cp );
    return 1;
  } else {
    nl_assert( ncf > 0 && cp + ncf <= nc );
    cp += ncf;
    return 0;
  }
}
