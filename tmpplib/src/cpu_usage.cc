#include "cpu_usage.h"

cpu_usage::cpu_usage() {
  last_sutime = 0;
  fd = -1;
}

cpu_usage::~cpu_usage() {
  if (fd >= 0) close(fd);
}

/**
 * Established the connection with the idle process.
 * This is separate from the constructor to facilitate
 * error reporting.
 */
void cpu_usage::init() {
  fd = open( "/proc/1/as", O_RDONLY );
  if ( fd < 0 ) nl_error( 3, "Unable to open /proc/1/as" );
}

/**
 * @param rate The rate at which this function is called in Hz.
 * @return Percent CPU utilization since the last call.
 */
unsigned char cpu_usage::report( double rate ) {
  procfs_status my_status;
  _Uint64t this_sutime;
  unsigned char rv = 0;

  my_status.tid = 1;
  devctl( fd, DCMD_PROC_TIDSTATUS, &my_status, sizeof(my_status), NULL );
  if (last_sutime) {
    double pct;
    _Uint64t dt = my_status.sutime - last_sutime;
    pct = 100. - rate * dt * 100e-9;
    rv = (pct < 0) ? 0 : ((unsigned char) pct);
  }
  last_sutime = my_status.sutime;
  return rv;
}
