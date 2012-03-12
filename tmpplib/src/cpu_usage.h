#ifndef CPU_USAGE_H_INCLUDED
#define CPU_USAGE_H_INCLUDED

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <devctl.h>
#include <sys/procfs.h>
#include "nortlib.h"


class cpu_usage {
  public:
    cpu_usage();
    ~cpu_usage();
    void init();
    unsigned char report(double rate);
  private:
    int fd;
    _Uint64t last_sutime;
};

#endif
