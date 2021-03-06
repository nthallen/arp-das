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

TM_Selectee::TM_Selectee(const char *name, void *data,
      unsigned short size) : Selectee() {
  init(name, data, size);
}

TM_Selectee::TM_Selectee() : Selectee() {
  TMid = 0;
}

void TM_Selectee::init(const char *name, void *data,
      unsigned short size) {
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
Cmd_Selectee::Cmd_Selectee( const char *name ) :
    Ser_Sel(tm_dev_name(name), O_RDONLY|O_NONBLOCK, 0) {}
Cmd_Selectee::Cmd_Selectee( const char *name, int bufsz ) :
    Ser_Sel(tm_dev_name(name), O_RDONLY|O_NONBLOCK, bufsz) {}

int Cmd_Selectee::ProcessData(int flag) {
  nl_error(-2, "Received NUL Quit Cmd");
  return 1;
}

/**
 * @param path The full path to the serial device. If path == NULL,
 * the fd will not be opened.
 * @param open_flags Flags from <fcntl.h> passed to open()
 * @param bufsz The size buffer to be allocated.
 */
Ser_Sel::Ser_Sel(const char *path, int open_flags, int bufsz )
    : Selectee() {
  sersel_init();
  init(path, open_flags, bufsz);
}

Ser_Sel::Ser_Sel() : Selectee() {
  sersel_init();
}

/**
 * @param path The full path to the serial device. If path == NULL,
 * the fd will not be opened.
 * @param open_flags Flags from <fcntl.h> passed to open()
 * @param bufsz The size buffer to be allocated.
 * bufsz must be large enough to include a complete data record plus
 * a terminating NUL character. fillbuf() guarantees that all input
 * is terminated with a NUL.
 */
void Ser_Sel::init(const char *path, int open_flags, int bufsz) {
  if (path == 0) {
    fd = -1;
  } else {
    fd = tm_open_name(path, NULL, open_flags);
    switch (open_flags & O_ACCMODE) {
      case O_RDWR:
      case O_RDONLY:
        flags |= Selector::Sel_Read;
        break;
    }
  }
  if (bufsz > 0) {
    buf = (unsigned char *)new_memory(bufsz);
    bufsize = bufsz;
  }
  termios_init = false;
}

void Ser_Sel::sersel_init() {
  fd = -1;
  buf = 0;
  bufsize = 0;
  nc = cp = 0;
  n_fills = n_empties = 0;
  n_eagain = n_eintr = 0;
  total_errors = 0;
  total_suppressed = 0;
  n_errors = 0;
  n_suppressed = 0;
}

/**
 * Frees the allocated buffer and reports statistics.
 */
Ser_Sel::~Ser_Sel() {
  if (buf) free_memory(buf);
  buf = 0;
  nl_error( 0, "n_fills: %d  n_empties: %d "
    "total_errors: %d total_suppressed: %d",
    n_fills, n_empties, total_errors, total_suppressed );
  nl_error( 0, "n_eagain: %d n_eintr: %d", n_eagain, n_eintr);
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
  int bitsflag;

  if ( fd < 0 ) return;
  init_termios();
  ss_termios.c_iflag = 0;
  ss_termios.c_lflag &= ~(ECHO|ICANON|ISIG|ECHOE|ECHOK|ECHONL);
  ss_termios.c_cflag = CLOCAL|CREAD;
  ss_termios.c_oflag &= ~(OPOST);
  ss_termios.c_ispeed = ss_termios.c_ospeed = baud;
  switch (bits) {
    case 5: bitsflag = CS5; break;
    case 6: bitsflag = CS6; break;
    case 7: bitsflag = CS7; break;
    case 8: bitsflag = CS8; break;
    default:
      nl_error( 3, "Invalid bits value: %d", bits );
  }
  ss_termios.c_cflag |= bitsflag;
  switch (par) {
    case 'n': bitsflag = 0; break;
    case 'e': bitsflag = PARENB; break;
    case 'o': bitsflag = PARENB|PARODD; break;
    case 'm': bitsflag = PARENB|PARODD|PARSTK; break;
    case 's': bitsflag = PARENB|PARSTK; break;
    default:
      nl_error( 3, "Invalid parity selector: '%c'", par );
  }
  ss_termios.c_cflag |= bitsflag;
  switch (stopbits) {
    case 1: break;
    case 2: ss_termios.c_cflag |= CSTOPB; break;
    default:
      nl_error(3, "Invalid number of stop bits: %d", stopbits );
  }
  ss_termios.c_cc[VMIN] = min;
  ss_termios.c_cc[VTIME] = time;
  if ( tcsetattr(fd, TCSANOW, &ss_termios) )
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
  return fillbuf(bufsize);
}

/**
 * @param N Limits the total number of characters in the buffer to N
 * N must be less than or equal to bufsize and should include space
 * for a trailing NUL.
 * Reads characters from the device, reporting any errors.
 * Guarantees that buf is NUL-terminated, and sets nc to the
 * total number of characters. Each call to fillbuf() increments
 * the n_fills counter, which is reported at termination
 * @return non-zero on error.
 */
int Ser_Sel::fillbuf(int N) {
  int i;
  if (!buf) nl_error(4, "Ser_Sel::fillbuf with no buffer");
  if (N > bufsize)
    nl_error(4, "Ser_Sel::fillbuf(N) N > bufsize: %d > %d",
      N, bufsize);
  if (nc > N-1) return 0;
  ++n_fills;
  i = read( fd, &buf[nc], bufsize - 1 - nc );
  if ( i < 0 ) {
    if ( errno == EAGAIN ) {
      ++n_eagain;
    } else if (errno == EINTR) {
      ++n_eintr;
    } else {
      nl_error( 2, "Error %d on read from serial port", errno );
      return 1;
    }
    return 0;
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

/**
 * Invokes fillbuf() until there is no input remaining.
 */
void Ser_Sel::flush_input() {
  do {
    nc = cp = 0;
    if (fillbuf()) return;
  } while (nc > 0);
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
      msg( 2, "Input was: '%s'", ascii_escape((char*)buf, nc) );
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
int Ser_Sel::not_found(unsigned char c) {
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
 * Parsing utility function to read in a hex integer starting
 * at the current position. Integer may be proceeded by optional
 * whitespace.
 * @param[out] hexval The integer value
 * @return zero if an integer was converted, non-zero if the current char is not a digit.
 */
int Ser_Sel::not_hex( unsigned short &hexval ) {
  hexval = 0;
  while (cp < nc && isspace(buf[cp]))
    ++cp;
  if (! isxdigit(buf[cp])) {
    if (cp < nc)
      report_err("No hex digits at col %d", cp);
    return 1;
  }
  while ( cp < nc && isxdigit(buf[cp]) ) {
    unsigned short digval = isdigit(buf[cp]) ? ( buf[cp] - '0' ) :
           ( tolower(buf[cp]) - 'a' + 10 );
    hexval = hexval * 16 + digval;
    ++cp;
  }
  return 0;
}

/**
 * Parsing utility function to read in a decimal integer starting
 * at the current position. Integer may be proceeded by optional
 * whitespace and an optional sign.
 * @param[out] val The integer value
 * @return zero if an integer was converted, non-zero if the current char is not a digit.
 */
int Ser_Sel::not_int( int &val ) {
  bool negative = false;
  // fillbuf() guarantees the buffer will be NUL-terminated, so any check
  // that will fail on a NUL is OK without checking the cp < nc
  while (cp < nc && isspace(buf[cp]))
    ++cp;
  if (cp < nc && buf[cp] == '-') {
    negative = true;
    ++cp;
  } else if (cp < nc && buf[cp] == '+') ++cp;
  if ( cp < nc && isdigit(buf[cp]) ) {
    val = buf[cp++] - '0';
    while ( cp < nc && isdigit(buf[cp]) ) {
      val = 10*val + buf[cp++] - '0';
    }
    if (negative) val = -val;
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
int Ser_Sel::not_str( const char *str_in, unsigned int len ) {
  unsigned int start_cp = cp;
  unsigned int i;
  const unsigned char *str = (const unsigned char *)str_in;
  if ( cp < 0 || cp > nc || nc < 0 || buf == 0 )
    nl_error( 4, "Ser_Sel precondition failed: "
      "cp = %d, nc = %d, buf %s",
      cp, nc, buf ? "not NULL" : "is NULL" );
  for (i = 0; i < len; ++i) {
    if ( cp >= nc ) {
      return 1; // full string is not present
    } else if ( str[i] != buf[cp] ) {
      report_err( "Expected string '%s' at column %d",
        ascii_escape(str_in, len), start_cp );
      return 1;
    }
    ++cp;
  }
  return 0;
}

int Ser_Sel::not_str( const char *str ) {
  return not_str(str, strlen(str));
}

int Ser_Sel::not_str(const std::string &s) {
  return not_str(s.c_str(), s.length());
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
  int start_cp = cp;
  int ncf;
  if ( cp < 0 || cp > nc || nc < 0 || nc >= bufsize || buf == 0 )
    msg( 4, "Ser_Sel precondition failed: "
      "cp = %d, nc = %d, bufsize = %d, buf %s",
      cp, nc, bufsize, buf ? "not NULL" : "is NULL" );

  if (cp >= nc) return 1;
  if (buf[cp] == '-') {
    if (++cp >= nc) return 1;
  }
  if (buf[cp] == '.') {
    if (++cp >= nc) return 1;
    if (isdigit(buf[cp])) {
      do { ++cp; }
        while (cp < nc && isdigit(buf[cp]));
    } else {
      return 1;
    }
  } else if (isdigit(buf[cp])) {
    do { ++cp; }
      while (cp < nc && isdigit(buf[cp]));
    if (cp < nc && buf[cp] == '.') {
      do { ++cp; }
        while (cp < nc && isdigit(buf[cp]));
    }
  } else {
    report_err("not_float expected [0-9.] at col %d", cp);
    return 1;
  }
  if (cp < nc && tolower(buf[cp]) == 'e') {
    if (++cp >= nc) return 1;
    if (buf[cp] == '+' || buf[cp] == '-') {
      if (++cp >= nc) return 1;
    }
    if (isdigit(buf[cp])) {
      do { ++cp; } while (cp < nc && isdigit(buf[cp]));
    } else {
      report_err("not_float expected digits at col %d", cp);
      return 1;
    }
  }

  val = strtof( (char *)&buf[start_cp], &endptr );
  if (endptr == (char *)&buf[cp]) {
    return 0;
  }
  ncf = endptr - (char *)buf;
  report_err("strtof at col %d ended at col %d, not %d",
    start_cp, ncf, cp);
  return 1;
}

void Ser_Sel::init_termios() {
  if (! termios_init) {
    if (tcgetattr(fd, &ss_termios)) {
      nl_error(2, "Error from tcgetattr: %s",
        strerror(errno));
    }
    termios_init = true;
  }
}

void Ser_Sel::update_tc_vmin(int new_vmin) {
  init_termios();
  if (new_vmin < 1) new_vmin = 1;
  if (new_vmin != ss_termios.c_cc[VMIN]) {
    ss_termios.c_cc[VMIN] = new_vmin;
    if (tcsetattr(fd, TCSANOW, &ss_termios)) {
      nl_error(2, "Error from tcsetattr: %s",
        strerror(errno));
    }
  }
}

void Ser_Sel::set_ohflow(bool ohflow_on) {
  init_termios();
  if (ohflow_on) {
    // Note that this code is QNX-specific
    ss_termios.c_cflag |= OHFLOW;
  } else {
    ss_termios.c_cflag &= ~OHFLOW;
  }
  if (tcsetattr(fd, TCSANOW, &ss_termios)) {
    nl_error(2, "Error from tcsetattr: %s",
      strerror(errno));
  }
}

const char *ascii_escape(const char *ibuf, int len) {
  static std::string s;
  char snbuf[8];
  int ix = 0, nb;
  s.clear();
  while (ix < len ) {
    unsigned char c = ibuf[ix++];
    if ( isprint(c) ) {
      s.push_back(c);
    } else {
      switch ( c ) {
        case '\n':
          s.push_back('\\');
          s.push_back('n');
          break;
        case '\r':
          s.push_back('\\');
          s.push_back('r');
          break;
        case '\t':
          s.push_back('\\');
          s.push_back('t');
          break;
        default:
          nb = snprintf( snbuf, 8, "\\x%02x", c);
          s.append(snbuf);
          break;
      }
    }
  }
  return s.c_str();
}

const char *ascii_escape(const std::string &s) {
  return ascii_escape(s.c_str(), s.length());
}

/**
 * Named differently to disambiguate from C version
 * in nortlib2. Invokes the C++ versions, which
 * have no inherent length limitation and can deal
 * with embedded NULs.
 */
const char *ascii_esc(const char *str) {
  return ascii_escape(str, strlen(str));
}
