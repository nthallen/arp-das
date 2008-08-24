#include <stdio.h>
#include "nortlib.h"
#include "subbus.h"

#ifdef __USAGE
%C
	Reads the switches from the system controller
	and writes the result as a single hex digit.
#endif

int main( void ) {
  unsigned int i;
  if ( load_subbus() == 0 )
	nl_error( 3, "Unable to load subbus library" );
  i = read_switches() & 0xF;
  printf( "%X\n", i );
  return 0;
}
