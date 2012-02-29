/**
 * \file Selectee.cc
 */
#include <unistd.h>
#include "Selector.h"
#include "nortlib.h"

/**
 * When fd and flags are known before construction.
 */
Selectee::Selectee(int fd_in, int flag) {
  flags = flag;
  fd = fd_in;
  Stor = 0;
}

/**
 * When fd is opened in the constructor.
 */
Selectee::Selectee() {
  flags = 0;
  fd = -1;
}

/**
 * Closes the fd if it is non-negative. Hence the fd value should be set to -1
 * if the fd is closed for any reason. And the fd should not be closed for any
 * reason until the event loop has terminated. This is clearly a feature that
 * needs updating.
 */
Selectee::~Selectee() {
  if ( fd >= 0 ) {
    close(fd);
    fd = -1;
  }
}

/**
 * Virtual method to allow Selectee to bid on the select() timeout
 * along with the Selector. The minimum timeout value is used.
 * @return a Timeout * indicating the requested timeout value or NULL.
 */
Timeout *Selectee::GetTimeout() {
  return NULL;
}
