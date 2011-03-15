#ifndef PTRH_H_INCLUDED
#define PTRH_H_INCLUDED

#include "nortlib.h"
#include "nl_assert.h"
#include "subbus.h"

typedef unsigned short USHRT;
class ptrh {
  public:
    ptrh( const char *id, USHRT base, USHRT C1, USHRT C2,
	  USHRT C3, USHRT C4, USHRT C5, USHRT C6);
    void check_coeffs();
    double SHT21T( USHRT S1 );
    double SHT21RH( USHRT S2 );
    double MS5607T( USHRT Dta, USHRT Dtb);
    double MS5607P( USHRT Dpa, USHRT Dpb);
  private:
    const char *id;
    USHRT base_addr;
    USHRT C[6];
    double C1d, C2d, C3d, C4d, C6d;
    unsigned long C5d;
    double Off, Sens;
    int check_coeff( int i, USHRT C);
    int stale;
};

#endif
