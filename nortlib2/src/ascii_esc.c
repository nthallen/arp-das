#include "nortlib.h"

/**
 * @return Pointer to a static buffer containing a NUL-terminated
 * rendition of the input string in printable characters. The output
 * string is limited to 82 characters, and is truncated if necessary.
 */
#define ESC_BUF_SIZE 80
char *ascii_escape(const char *ibuf) {
  static char ebuf[ESC_BUF_SIZE+3];
  int ix = 0, ox = 0;
  while (ibuf[ix] != '\0' && ox < ESC_BUF_SIZE ) {
    char c = ibuf[ix++];
    if ( isprint(c) ) {
      ebuf[ox++] = c;
    } else {
      switch ( c ) {
        case '\n':
          ebuf[ox++] = '\\';
          ebuf[ox++] = 'n';
          break;
        case '\r':
          ebuf[ox++] = '\\';
          ebuf[ox++] = 'r';
          break;
        case '\t':
          ebuf[ox++] = '\\';
          ebuf[ox++] = 't';
          break;
        default:
          ox += snprintf( ebuf+ox, 4, "\\x%02x", c);
          break;
      }
    }
  }
  ebuf[ox] = '\0';
  return ebuf;
}
