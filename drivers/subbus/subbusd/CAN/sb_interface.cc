/**
 * \file interface.cc
 */
#include <errno.h>
#include <string>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include "sb_interface.h"
#include "msg.h"
#include "nl_assert.h"
#include "nortlib.h" //#include "dasio/ascii_escape.h"
// #include "dasio/loop.h"

/**
 * I am assuming here that this level is low enough that I don't
 * need to have two separate invocations, once for deferred
 * initialization.
 * I am currently choosing to avoid providing an output buffer
 * here.
 * @param name An identifier for the interface to be used when
 * reporting statistics.
 * @param bufsz The size of the input buffer
 */
sb_interface::sb_interface(const char *name, int bufsz) {
  iname = name;
  nc = cp = 0;
  bufsize = 0;
  buf = 0;
  fd = -1;
  // flags = 0;
  // signals = 0;
  // ELoop = 0;
  // n_fills = n_empties = n_eagain = n_eintr = 0;
  n_errors = 0;
  n_suppressed = 0;
  total_errors = 0;
  total_suppressed = 0;
  qerr_threshold = 5;
  // wiov = 0;
  // n_wiov = 0;
  // ref_count = 0;
  binary_offset = -1;
  binary_mode = false;
  set_ibufsize(bufsz);
}

sb_interface::~sb_interface() {
  // msg(0, "Destructing sb_interface %s", iname);
  if (fd >= 0) {
    close();
  }
  set_ibufsize(0);
}

#if 0
bool sb_interface::serialized_signal_handler(uint32_t signals_seen) {
  msg(MSG, "received signals: %08X", signals_seen);
  return true;
}

/**
 * Virtual method to allow an interface to bid on the select() timeout
 * along with the Loop. The minimum timeout value is used.
 * @return a Timeout * indicating the requested timeout value or NULL.
 */
Timeout *sb_interface::GetTimeout() {
  return &TO;
}

bool sb_interface::ProcessData(int flag) {
  // msg(0, "%s: sb_interface::ProcessData(%d)", iname, flag);
  if ((flags & flag & gflag(0)) && tm_sync())
    return true;
  if ((flags&Fl_Read) && (flags&flag&(Fl_Read|Fl_Timeout))) {
    if (fillbuf(bufsize, flag)) return true;
    if (fd < 0) return false;
    cp = 0;
    if (protocol_input()) return true;
  }
  if ((flags & flag & Fl_Write) && iwrite_check())
    return true;
  if ((flags & flag & Fl_Except) && protocol_except())
    return true;
  if ((flags & flag & Fl_Timeout) && TO.Expired() && protocol_timeout())
    return true;
  if (TO.Set()) {
    flags |= Fl_Timeout;
  } else {
    flags &= ~Fl_Timeout;
  }
  return false;
}

void sb_interface::adopted() {}

void sb_interface::dereference(sb_interface *P) {
  if (--P->ref_count == 0) {
    delete(P);
  }
}

bool sb_interface::iwritev(struct iovec *iov, int nparts) {
  wiov = iov;
  n_wiov = nparts;
  return iwrite_check();
}

/**
 * Sets up a write of nc bytes from the buffer pointed to by str.
 * If the write cannot be accomplished immediately, the information
 * is saved and handled transparently. The caller is
 * responsible for allocating the output buffer(s) and ensuring
 * they are not overrun.
 * The caller can check obuf_empty() to determine whether the
 * write has completed.
 * @param str Pointer to the output buffer
 * @param nc The total number of bytes in the output buffer
 * @param cp The starting offset within the output buffer
 * @return true if a fatal error occurs
 */
bool sb_interface::iwrite(const char *str, unsigned int nc, unsigned int cp) {
  if (fd < 0) {
    msg(MSG_EXIT_ABNORM, "%s: Connection closed unexpectedly", iname);
  }
  nl_assert(obuf_empty());
  pvt_iov.iov_base = (void *)(str+cp);
  pvt_iov.iov_len = nc - cp;
  return iwritev(&pvt_iov, 1);
  // obuf = (unsigned char *)str;
  // onc = nc;
  // ocp = cp;
  // return iwrite_check();
}

bool sb_interface::iwrite_check() {
  bool rv = false;
  if (!obuf_empty()) {
    int ntr = writev(fd, wiov, n_wiov);
    if (ntr < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
      flags &= ~Fl_Write;
      return(iwrite_error(errno));
    }
    if (ntr > 0) {
      int ntrr = ntr;
      while (ntr > 0 && n_wiov > 0) {
        if (ntr >= wiov->iov_len) {
          ntr -= wiov->iov_len;
          if (--n_wiov > 0)
            ++wiov;
        } else {
          wiov->iov_len -= ntr;
          wiov->iov_base = (void*)(((const char *)(wiov->iov_base))+ntr);
          ntr = 0;
        }
      }
      rv = iwritten(ntrr);
    }
  }
  flags = obuf_empty() ?
    flags & ~Fl_Write :
    flags | Fl_Write;
  return rv;
}

bool sb_interface::iwrite(const std::string &s) {
  return iwrite(s.c_str(), s.length());
}

bool sb_interface::iwrite(const char *str) {
  return iwrite(str, strlen(str));
}

void sb_interface::iwrite_cancel() {
  n_wiov = 0;
}

/**
 * The default implementation does nothing.
 */
bool sb_interface::iwritten(int nb) { return false; }

/**
 * The default implementation returns true.
 */
bool sb_interface::iwrite_error(int my_errno) {
  msg(MSG_ERROR, "%s: write error %d: %s", iname, my_errno, strerror(my_errno));
  return true;
}

/**
 * The default function returns true.
 */
bool sb_interface::read_error(int my_errno) {
  msg(MSG_ERROR, "%s: read error %d: %s", iname, my_errno, strerror(my_errno));
  return true;
}
#endif

/**
 * The default reports unexpected input and returns false;
 */
bool sb_interface::protocol_input() {
  if (nc > 0)
    report_err("Unexpected input");
  return false;
}

/**
 * The default does nothing and returns false.
 */
bool sb_interface::protocol_timeout() {
  return false;
}

#if 0
/**
 * The default does nothing and returns false.
 */
bool sb_interface::protocol_except() {
  return false;
}

/**
 * The default does nothing and returns false.
 */
bool sb_interface::tm_sync() {
  return false;
}

bool sb_interface::process_eof() {
  return true;
}
#endif

void sb_interface::close() {
  if (fd >= 0) {
    ::close(fd);
    fd = -1;
    // TO.Clear();
    // flags &= ~(Fl_Write|Fl_Read|Fl_Except|Fl_Timeout);
  }
}

void sb_interface::set_ibufsize(int bufsz) {
  if (bufsize != bufsz) {
    if (buf) free_memory(buf);
    bufsize = bufsz;
    buf = bufsize ? (unsigned char *)new_memory(bufsize) : 0;
  }
}

#if 0
bool sb_interface::fillbuf(int N, int flag) {
  int nb_read;
  if (!buf) msg(MSG_EXIT_ABNORM, "Ser_Sel::fillbuf with no buffer");
  if (N > bufsize)
    msg(MSG_EXIT_ABNORM, "Ser_Sel::fillbuf(N) N > bufsize: %d > %d",
      N, bufsize);
  if (nc >= N+binary_offset) return false;
  ++n_fills;
  nb_read = read( fd, &buf[nc], N + binary_offset - nc );
  if ( nb_read < 0 ) {
    if ( errno == EAGAIN ) {
      ++n_eagain;
    } else if (errno == EINTR) {
      ++n_eintr;
    } else {
      return read_error(errno);
    }
    return false;
  } else if (nb_read == 0 && (flag & Fl_Read)) {
    close();
    return process_eof();
  }
  nc += nb_read;
  if (!binary_mode) buf[nc] = '\0';
  return false;
}
#endif

void sb_interface::consume(int nchars) {
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

bool sb_interface::not_suppressing() {
  return n_suppressed == 0 && ( qerr_threshold < 0 || n_errors < qerr_threshold );
}

void sb_interface::report_err( const char *fmt, ... ) {
  ++total_errors;
  // Here we're counting up only if there is a threshold and we're still under it
  if ( qerr_threshold >= 0 && n_errors < qerr_threshold )
    ++n_errors;
  // Here we display if there is no threshold or we're under the threshold
  if ( not_suppressing() ) {
    va_list args;

    va_start(args, fmt);
    msgv( MSG_ERROR, fmt, args );
    va_end(args);
    if (nc)
      msg(MSG_ERROR, "%s: Input was: '%s'", iname, ascii_escape() );
  } else {
    if ( !n_suppressed )
      msg(MSG_ERROR, "%s: Error threshold reached: suppressing errors", iname);
    ++n_suppressed;
    ++total_suppressed;
  }
}

#if 0
void sb_interface::signal(int signum, bool handle) {
  if (ELoop == 0) {
    msg(MSG_FATAL, "%s: signal() called before Loop::add_child()", iname);
  }
  if ((1 <= signum) && (signum <= 32)) {
    if (handle) {
      /* Set the bit in question. */
      signals |= 1 << (signum - 1);
      ELoop->signal(signum, loop_signal_handler);
    } else {
      /* Clear the bit in question. */
      signals &= ~(1 << (signum - 1));
      ELoop->signal(signum, SIG_DFL);
    }
  } else {
    msg(MSG_FATAL, "unknown signal: %u", signum);
  }
}
#endif

/**
 * This version does not support a full binary mode,
 * which is OK for subbus, where everything is ASCII.
 */
const char *sb_interface::ascii_escape() {
  // return ::ascii_escape((char*)buf, nc);
  nl_assert(nc < bufsize);
  buf[nc] = '\0';
  return ::ascii_escape((char*)buf);
}

/**
 * Signals that a successful protocol transfer occurred,
 * reducing the qualified error count, potentially reenabling
 * logging of errors.
 */
void sb_interface::report_ok(int nchars) {
  if ( n_errors > 0 ) {
    if ( --n_errors <= 0 && n_suppressed ) {
      msg( 0, "Error recovery: %d error messages suppressed", n_suppressed );
      n_suppressed = 0;
    }
  }
  if (nchars > 0) {
    consume(nchars);
  }
}

bool sb_interface::not_hex( uint16_t &hexval ) {
  hexval = 0;
  while (cp < nc && isspace(buf[cp]))
    ++cp;
  if (! isxdigit(buf[cp])) {
    if (cp < nc)
      report_err("%s: No hex digits at col %d", iname, cp);
    return true;
  }
  while ( cp < nc && isxdigit(buf[cp]) ) {
    uint16_t digval = isdigit(buf[cp]) ? ( buf[cp] - '0' ) :
           ( tolower(buf[cp]) - 'a' + 10 );
    hexval = hexval * 16 + digval;
    ++cp;
  }
  return false;
}

bool sb_interface::not_nhexdigits(int n, uint16_t &value) {
  int i = n;
  value = 0;
  while ( i > 0 && cp < nc && isxdigit(buf[cp])) {
    uint16_t digval = isdigit(buf[cp]) ? ( buf[cp] - '0' ) :
           ( tolower(buf[cp]) - 'a' + 10 );
    value = value*16 + digval;
    --i; ++cp;
  }
  if (i > 0) {
    if (cp < nc)
      report_err("%s: Expected %d hex digits at column %d", iname, n, cp+i-n);
    return true;
  }
  return false;
}

bool sb_interface::not_str( const char *str_in, unsigned int len ) {
  unsigned int start_cp = cp;
  unsigned int i;
  const unsigned char *str = (const unsigned char *)str_in;
  if ( cp < 0 || cp > nc || nc < 0 || buf == 0 )
    msg( MSG_EXIT_ABNORM, "sb_interface precondition failed: "
      "cp = %d, nc = %d, buf %s",
      cp, nc, buf ? "not NULL" : "is NULL" );
  for (i = 0; i < len; ++i) {
    if ( cp >= nc ) {
      return true; // full string is not present
    } else if ( str[i] != buf[cp] ) {
      report_err("%s: Expected string '%s' at column %d", iname,
        ::ascii_escape(str_in /* , len */), start_cp );
      return true;
    }
    ++cp;
  }
  return false;
}

bool sb_interface::not_str( const char *str ) {
  return not_str(str, strlen(str));
}

bool sb_interface::not_str(const std::string &s) {
  return not_str(s.c_str(), s.length());
}
