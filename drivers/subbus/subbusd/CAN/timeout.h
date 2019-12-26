/** @file timeout.h */

#ifndef TIMEOUT_H_INCLUDED
#define TIMEOUT_H_INCLUDED
#include <time.h>

class Timeout {
  public:
    Timeout();
    /**
     * Specifies a desired timeout relative to the current time. The event loop
     * will subtract the then-current time to determine the correct relative
     * timeout value.
     * @param secs Seconds
     * @param msecs Milleseconds
     */
    void Set( time_t secs, long msecs );
    /**
     * Clears any current timeout setting.
     */
    void Clear();
    /**
     * @return True if a timeout value is set, regardless of whether
     * the time has expired or not.
     */
    bool Set();
    /**
     * @return True if a timeout value is set and has expired.
     */
    bool Expired();
    // struct timespec when;
};

#endif

