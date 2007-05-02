/* tm_open_name.c */
#include <limits.h>
#include <errno.h>
#include <string.h>
#include "tm.h"
#include "nortlib.h"

int tm_open_name( const char * name, const char *node, int flags ) {
  char fullname[PATH_MAX], *fname;
  int fd;
  
  if ( node != NULL ) fname = name;
  else {
	int nb;
	nb = snprintf( fullname, PATH_MAX, "/net/%s%s", node, name );
	if ( nb >= PATH_MAX ) {
	  if (nl_response)
		nl_error(nl_response,
		  "node-qualified path is too long: /net/%s%s",
		  node, name );
	  errno = ENAMETOOLONG;
	  return -1;
	}
	fname = fullname;
  }
  fd = open( fname, flags );
  if ( fd < 0 ) {
	if ( nl_response )
	  nl_error( nl_response,
	    "Error %d opening %s: %s", errno, fname, strerror(errno)
		);
  }
  return fd;
}
