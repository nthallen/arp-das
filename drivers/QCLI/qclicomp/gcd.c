#include "qclicomp.h"

long gcd( long a, long b ) {
  long int r;
  if ( a < 0 ) a = -a;
  if ( b < 0 ) b = -b;
  while ( b != 0 ) {
    r = a %  b;
	a = b;
	b = r;
  }
  return a;
}
