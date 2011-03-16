#include <stdio.h>
#include "ptrh.h"

ptrh ptrhA = ptrh( "DACS PTRH", 0x300, 0xB28E, 0x9548, 0x6EE9, 0x610A, 0x812C, 0x6C80 );
ptrh ptrhB = ptrh( "SPV PTRH", 0x320, 0xB28E, 0x9548, 0x6EE9, 0x610A, 0x812C, 0x6C80 );
ptrh ptrhC = ptrh( "PTRH #3", 0x340, 0xB28E, 0x9548, 0x6EE9, 0x610A, 0x812C, 0x6C80 );

void report( const char *id, double Ta, double RH, double Tb, double P ) {
  printf("Reporting %s:\n", id );
  printf("  Ta = %.2lf\n", Ta);
  printf("  RH = %.2lf\n", RH);
  printf("  Tb = %.2lf\n", Tb);
  printf("  P  = %.2lf\n", P);
}

void read_report( ptrh *dev ) {
  USHRT status;
  if ( read_ack(dev->base_addr, &status) ) {
    double Ta, RH, Tb, P;
    USHRT T1, RHr, T2a, Pa;
    unsigned char T2b, Pb;
    printf("\nTesting %s:\n", dev->id);
    if (status != 0x0FFF )
      printf("  Status readback was 0x%04X\n", status);
    if (status != 0) {
      dev->check_coeffs();
      T1 = dev->SHT21T();
      RHr = dev->SHT21RH();
      T2a = dev->MS5607Ta();
      T2b = dev->MS5607Tb();
      Pa = dev->MS5607Pa();
      Pb = dev->MS5607Pb();
      Ta = dev->SHT21T(T1);
      RH = dev->SHT21RH(RHr);
      Tb = dev->MS5607T(T2a, T2b);
      P = dev->MS5607P(Pa, Pb);
      report( dev->id, Ta, RH, Tb, P );
    }
  }
}

int main( int argc, char **argv ) {
  int subbus_id = load_subbus();
  if ( subbus_id == 0 ) {
    double Ta, RH, Tb, P;
    Ta = ptrhA.SHT21T(0x61C4);
    RH = ptrhA.SHT21RH(0x5D86);
    Tb = ptrhA.MS5607T(0x62E0, 0x0081);
    P = ptrhA.MS5607P(0x38FE, 0x0059);
    report("Canned Values", Ta, RH, Tb, P);
  } else {
    read_report(&ptrhA);
    read_report(&ptrhB);
    read_report(&ptrhC);
  }
  return 0;
}

