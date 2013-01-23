/**
   cpu_usage module for use with telemetry data collection.
   This is based on QNX6 documentation:
    QNX Realtime Operating System
      Programmer's Guide
        Processes
          Controlling processes via the /proc filesystem
            DCMD_PROC_SYSINFO
            DCMD_PROC_TIDSTATUS
 */
#include "cpu_usage.h"

cpu_usage::cpu_usage() {
  last_sutime = 0;
  PCT = 0;
  fd = -1;
}

cpu_usage::~cpu_usage() {
  if (fd >= 0) close(fd);
  if (PCT) free_memory(PCT);
  if (last_sutime) free_memory(last_sutime);
}

/**
 * Established the connection with the idle process.
 * This is separate from the constructor to facilitate
 * error reporting.
 */
void cpu_usage::init(unsigned int n_cpus) {
  unsigned i;
  fd = open( "/proc/1/as", O_RDONLY );
  if ( fd < 0 ) nl_error( 3, "Unable to open /proc/1/as" );
  n_cores = n_cpus;
  PCT = (unsigned char*)new_memory(n_cpus);
  last_sutime = (_Uint64t*)new_memory(sizeof(_Uint64t)*n_cpus);
  for (i = 0; i < n_cpus; ++i) {
    PCT[i] = 0;
    last_sutime[i] = 0;
  }
  // Now call DCMD_PROC_SYSINFO to verify the number of processors
  { procfs_sysinfo si;
    devctl(fd, DCMD_PROC_SYSINFO, &si, sizeof(procfs_sysinfo), NULL);
    if (si.num_cpu < n_cores) {
      nl_error(1, "Processor reports only %d cores, not %d",
        si.num_cpu, n_cores);
      for (i = si.num_cpu; i < n_cores; ++i)
        PCT[i] = 255;
      n_cores = si.num_cpu;
    } else if (si.num_cpu > n_cores) {
      nl_error(1, "Processor reports %d cores, only monitoring %d",
        si.num_cpu, n_cores);
    }
  }
  report(1);
}

/**
 * @param rate The rate at which this function is called in Hz.
 * @return Percent CPU utilization since the last call.
 */
unsigned char cpu_usage::report( double rate ) {
  unsigned i;

  for (i = 0; i < n_cores; ++i) {
    procfs_status my_status;
    _Uint64t this_sutime;
    my_status.tid = i+1;
    devctl( fd, DCMD_PROC_TIDSTATUS, &my_status, sizeof(my_status), NULL );
    if (last_sutime[i]) {
      double pct;
      _Uint64t dt = my_status.sutime - last_sutime[i];
      pct = 100. - rate * dt * 100e-9;
      PCT[i] = (pct < 0) ? 0 : ((unsigned char) pct);
    }
    last_sutime[i] = my_status.sutime;
  }
  return PCT[0];
}
