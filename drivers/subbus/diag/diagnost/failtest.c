/* failtest.c
 * $Log$
 * Revision 1.1  2008/08/24 15:35:38  ntallen
 * Diagnostics I want to port
 *
 * Revision 1.1  2008/07/23 13:07:49  nort
 * Uncommitted changes and imports
 *
 *
 */
#include <stdlib.h>
#include "nortlib.h"
#ifdef __QNXNTO__
  #include <string.h>
  #include <errno.h>
  #include <sys/neutrino.h>
  #include <hw/inout.h>
  #define outp(x,y) out8(x,y)
#else
  #include <conio.h>
#endif
char failtest_c_rcsid[] = "$Id$";

#ifdef __USAGE
%C	n
	Where n is one of:
	0 - Turn off both fail indicators
	1 - Turn on just operate fail indicator
	2 - Turn on just record fail indicator
	3 - Turn on both fail indicators
#endif

int main(int argc, char **argv) {
  int mask;

  if (argc < 2) nl_error( 3, "Must specify fail value" );
  else {
    #ifdef __QNXNTO__
      if (ThreadCtl(_NTO_TCTL_IO,0) == -1 )
          nl_error( 3, "Error requesting I/O priveleges: %s", strerror(errno) );
    #endif
    mask = atoi(argv[1]);
    outp(0x319, 0); /* tick to turn of 2-minute failure */
    outp(0x311, 0);
    outp(0x317, mask & 3);
  }
  return(0);
}
