#include <stdio.h>
#include "ptrh.h"

ptrh ptrhA = ptrh( "DACS", 0x300, 0xFFFF, 0x9E96, 0x7106, 0x673E, 0x80F5, 0x6C35 );

int main( int argc, char **argv ) {
  double Ta, RH, Tb, P;
  Ta = ptrhA.SHT21T(0x6768);
  RH = ptrhA.SHT21RH(0x2772);
  Tb = ptrhA.MS5607T(0xD9FE, 0x0082);
  P = ptrhA.MS5607P(0xA4F2, 0x005A);
  printf("Ta = %.2lf\n", Ta);
  printf("RH = %.2lf\n", RH);
  printf("Tb = %.2lf\n", Tb);
  printf("P  = %.2lf\n", P);
  return 0;
}

