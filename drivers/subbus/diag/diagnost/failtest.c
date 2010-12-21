/* failtest.c
 * $Log$
 * Revision 1.2  2008/09/09 13:20:44  ntallen
 * Port failtest to NTO
 *
 * Revision 1.1  2008/08/24 15:35:38  ntallen
 * Diagnostics I want to port
 *
 * Revision 1.1  2008/07/23 13:07:49  nort
 * Uncommitted changes and imports
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "nortlib.h"
#include "subbus.h"


#ifdef __USAGE
%C	n
	Where n is one of:
	0 - Turn off both fail indicators
	1 - Turn on just operate fail indicator
	2 - Turn on just record fail indicator
	3 - Turn on both fail indicators
#endif

int main( int argc, char **argv ) {
  unsigned int mask, rv;
  if ( load_subbus() == 0 )
	nl_error( 3, "Unable to load subbus library" );
  if ( argc < 2 ) nl_error(3, "Must specify fail value" );
  mask = atoi(argv[1]);
  rv = set_failure( mask );
  printf("set_failure(%d) returned %d\n", mask, rv );
  return 0;
}
