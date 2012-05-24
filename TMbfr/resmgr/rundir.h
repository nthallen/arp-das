/* rundir.h */
#ifndef RUNDIR_H_INCLUDED
#define RUNDIR_H_INCLUDED

#include <sys/types.h>

#ifdef __cplusplus
  extern "C" {
#endif
  
extern void mkfltdir(const char *dir, uid_t flt_uid, gid_t flt_gid);
extern void setup_rundir(void);
extern void delete_rundir(void);
#define RUNDIR "/var/huarp/run"

#ifdef __cplusplus
  };
#endif

#endif
