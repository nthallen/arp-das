#include <stdio.h>
#include <stdlib.h>
#include "nortlib.h"
#include "subbus.h"

#ifdef __USAGE
%C	[mask]
	Reads the switches from the system controller
	and writes the result as two hex digits.
	The mask is specified using standard C syntax,
	and the default value is 0xF. Unmasked bits
	are set to 1 (flight mode!)
#endif

int main( int argc, char **argv ) {
  unsigned short sw;
  unsigned short mask = 0xF;

  if ( argc > 1 ) {
    unsigned long tmp_mask = strtoul(argv[1],NULL,0);
    mask = (unsigned short)tmp_mask;
  }

  if ( load_subbus() == 0 )
	nl_error( 3, "Unable to load subbus library" );
  sw = ((read_switches() & mask) | ~mask) & 0xFF;
  printf( "%02X\n", sw );
  return 0;
}
