/*
  mamba_temp.cc
  
  read MSR during ISR and queue data to thread
*/
#include <time.h>
#include "nortlib.h"

int main(int argc, char **argv) {
  struct timespec res;
  if (clock_getres(CLOCK_REALTIME, &res))
    nl_error(3, "clock_getres() returned an error");
  if (res.tv_sec)
    nl_error(1, "Clock resolution is long!: %ld sec", res.tv_sec);
  nl_error(0, "Clock resolution is %ld ns", res.tv_nsec);
  return 0;
}
