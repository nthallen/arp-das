#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "nortlib.h"
#include "tm.h"

// ### I suspect this needs to be rewritten

int TM_open_stream( int optimal ) {
  char *devname;

  devname = tm_dev_name( optimal ? "TM/DCo" : "TM/DCf" );
  if ( devname == 0 ) {
    nl_error( nl_response, "No memory in TM_open_stream" );
    return 1;
  }
  TM_fd = open( devname, O_RDONLY );
  if ( TM_fd < 0 ) {
    nl_error( nl_response, "Error opening '%s': %s",
      devname, strerror(errno) );
    return 1;
  }
  return 0;
}

/*
=Name TM_open_stream(): Open TM Stream
=Subject TM Functions
=Synopsis
#include "tm.h"
int TM_open_stream( int optimal );

=Description

  TM_open_stream() opens the TM stream at /dev/huarp/$Exp/TM/DC*
  for reading, where $Exp is the value of the environment
  variable 'Experiment'. If optimal is non-zero, TM/DCo is
  opened to signal that replies should fill the request buffers.
  If optimal is zero, requests will return as soon as there is
  at least a row of data to report.  Any errors are fatal
  unless =nl_response= is set to a lower level.

=Returns

  Returns zero on success, non-zero on failure, but only if
  =nl_response= is set to a non-fatal level.

=SeeAlso

  =TM Functions=.
=End
*/

