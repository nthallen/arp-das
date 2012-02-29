/**
 * \file Selectee.cc
 */
#include <unistd.h>
#include "Selector.h"
#include "nortlib.h"

Selectee::Selectee(int fd_in, int flag) {
  flags = flag;
  fd = fd_in;
  Stor = 0;
}

Selectee::Selectee() {
  flags = 0;
  fd = -1;
}

Selectee::~Selectee() {
  if ( fd >= 0 ) {
    close(fd);
    fd = -1;
  }
}

Timeout *Selectee::GetTimeout() {
  return NULL;
}
