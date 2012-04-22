#ifndef TIMEOUT_H_INCLUDED
#define TIMEOUT_H_INCLUDED

#include <time.h>
#include <sys/select.h>

class Timeout {
  public:
    Timeout();
    // Timeout( time_t secs, long msecs );
    void Set( time_t secs, long msecs );
    void Clear();
    bool Set();
    bool Expired();
    struct timespec when;
};

class TimeoutAccumulator {
  public:
    TimeoutAccumulator();
    void Set( Timeout * );
    void Set_Min( Timeout * );
    struct timeval *timeout_val();
  private:
    struct timespec when;
    struct timeval how_soon;
};

#endif
