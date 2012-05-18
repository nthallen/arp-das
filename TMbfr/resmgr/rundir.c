#include <grp.h>
#include <pwd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include "rundir.h"

void mkfltdir(const char *dir, uid_t flt_uid, gid_t flt_gid) {
  struct stat buf;
  if ( stat( dir, &buf) == -1 ) {
    if (errno == ENOENT) {
      if ( mkdir( dir, 02775 ) == -1)
        nl_error(3,"Error creating directory %s: %s", dir, strerror(errno));
    } else {
      nl_error(3,"Error on stat(%s): %s", dir, strerror(errno));
    }
  } else if (! S_ISDIR(buf.st_mode)) {
    // check to make sure it's a directory
    nl_error(3, "%s is not a directory", dir);
  } else {
    if (chmod(dir, 02775) == -1)
      nl_error(3, "Error on chmod(%s): %s", dir, strerror(errno));
  }
  if ( chown( dir, flt_uid, flt_gid) == -1)
    nl_error(3,"Error chowning directory %s: %s", dir, strerror(errno));
}

void setup_rundir(void) {
  struct group *flt_grp;
  struct passwd* flt_user;
  uid_t flt_uid;
  gid_t flt_gid;
  const char *rundir;
  const char *Exp;
  char runexpdir[80];
  int nb;

  flt_user = getpwnam("flight");
  if (flt_user == NULL) nl_error(3, "No flight user" );
  flt_grp = getgrnam("flight");
  if (flt_grp == NULL) nl_error(3, "No flight group" );
  flt_uid = flt_user->pw_uid
  flt_gid = flt_grp->gr_gid

  rundir = "/var/huarp/run";
  mkfltdir(rundir, flt_uid, flt_gid);

  Exp = getenv("Experiment");
  if (Exp == NULL) Exp = "none";
  nb = snprintf(runexpdir, 80, "%s/%s", rundir, Exp);
  nl_assert(nb < 80);
  if (stat(runexpdir, &buf) != -1 || errno != ENOENT) {
    char rmcmd[80];
    int nb = snprintf(rmcmd, 80, "/bin/rm -rf %s", runexpdir);
    nl_assert(nb < 80);
    int rv = system(rmcmd);
    if (rv == -1) {
      nl_error(3, "system(%s) failed: %s", rmcmd, strerror(errno));
    } else if ( WEXITSTATUS(rv) ) {
      nl_error(3, "system(%s) returned non-zero status: %d", WEXITSTATUS(rv));
    }
    if (stat(runexpdir, &buf) != -1 || errno != ENOENT) {
      nl_error(3, "Failed to delete runexpdir %s", runexpdir);
    }
  } // else the directory does not exist
  mkfltdir(runexpdir, flt_uid, flt_gid);
}
