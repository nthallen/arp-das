#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "nortlib.h"
#include "tm.h"

int TM_fd = -1;
char *TM_buf;

int TM_open_stream( int write, int nonblocking ) {
  char *exp, *devname;
  int namelen, splen, mode;

  exp = getenv("Experiment");
  if ( exp == NULL ) {
    nl_error( nl_response, "Experiment undefined in TM_open_stream" );
    return 1;
  }
  namelen = strlen(TM_DEV_BASE) + strlen(exp) + strlen(TM_DEV_SUFFIX) + 2;
  devname = (char *)alloca( namelen+2 );
  if ( devname == 0 ) {
    nl_error( nl_response, "No memory in TM_open_stream" );
    return 1;
  }
  splen = snprintf( devname, namelen+2, "%s/%s/%s",
                   TM_DEV_BASE, exp, TM_DEV_SUFFIX );
  if ( splen > namelen )
    nl_error( 4, "Overrun on snprintf in TM_open_stream" );
  mode = write ? O_WRONLY : O_RDONLY;
  if ( nonblocking ) mode |= O_NONBLOCK;
  TM_fd = open( devname, mode );
  if ( TM_fd < 0 ) {
    nl_error( nl_response, "Error opening '%s': %s",
      devname, strerror(errno) );
    return 1;
  }
  /* Do I need to allocate TM_buf for writing? */
  if ( ! write ) {
	TM_buf = (char *) malloc( TMBUFSIZE );
	if ( TM_buf == 0 ) {
	  nl_error( nl_response, "No memory for TM_buf in TM_open_stream" );
	  return 1;
	}
  }
  return 0;
}

/*
=Name TM_open_stream(): Open TM Stream
=Subject TM Functions
=Synopsis
#include "tm.h"
int TM_open_stream( int write, int nonblocking );

=Description

  TM_open_stream() opens the TM stream at /dev/huarp/$Exp/TM
  for reading, where $Exp is the value of the environment
  variable 'Experiment'. If nonblocking is non-zero, the file
  descriptor is set to non-blocking mode. Any errors are fatal
  unless =nl_response= is set to a lower level.

=Returns

  Returns zero on success, non-zero on failure, but only if
  =nl_response= is set to a non-fatal level.

=SeeAlso

  =TM Functions=.
=End
*/

