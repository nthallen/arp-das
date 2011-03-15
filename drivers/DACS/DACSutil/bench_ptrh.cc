#include <stdio.h>
#include "ptrh.h"

ptrh ptrhA = ptrh( "DACS", 0x300, 0xB28E, 0x9548, 0x6EE9, 0x610A, 0x812C, 0x6C80 );

int main( int argc, char **argv ) {
  double Ta, RH, Tb, P;
  Ta = ptrhA.SHT21T(0x61C4);
  RH = ptrhA.SHT21RH(0x5D86);
  Tb = ptrhA.MS5607T(0x62E0, 0x0081);
  P = ptrhA.MS5607P(0x38FE, 0x0059);
  printf("Ta = %.2lf\n", Ta);
  printf("RH = %.2lf\n", RH);
  printf("Tb = %.2lf\n", Tb);
  printf("P  = %.2lf\n", P);
  return 0;
}

