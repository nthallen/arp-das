#include "qclicomp.h"

long int gcd( long int a, long int b ) {
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
