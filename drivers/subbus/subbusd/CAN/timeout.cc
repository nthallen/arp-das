/** @file timeout.cc */
#include "timeout.h"
#include "sb_CAN.h"

Timeout::Timeout() {
  Clear();
}

void Timeout::Set(time_t secs, long msecs) {
  timeout_set(secs, msecs);
}

bool Timeout::Set() {
  return timeout_is_set;
}

void Timeout::Clear() {
  timeout_clear();
}
