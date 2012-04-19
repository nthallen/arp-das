#ifndef PTRH_H_INCLUDED
#define PTRH_H_INCLUDED

#include "nortlib.h"
#include "nl_assert.h"
#include "subbus.h"

typedef unsigned short USHRT;
class ptrhm {
  public:
    ptrhm( const char *id, USHRT base, USHRT C1, USHRT C2,
           USHRT C3, USHRT C4, USHRT C5, USHRT C6);
    void check_coeffs();
    void acquire();
    inline USHRT Status() { return P[0]; }
    inline USHRT SHT21T() { return P[1]; }
    inline USHRT SHT21RH() { return P[2]; }
    inline USHRT MS5607Ta() { return P[5]; }
    inline unsigned char MS5607Tb() { return P[6]; }
    inline USHRT MS5607Pa() { return P[3]; }
    inline unsigned char MS5607Pb() { return P[4]; }
    double SHT21T( USHRT S1 );
    double SHT21RH( USHRT S2 );
    double MS5607T( USHRT Dta, USHRT Dtb);
    double MS5607P( USHRT Dpa, USHRT Dpb);
  private:
    const char *id;
    USHRT base_addr;
    USHRT C[6];
    USHRT P[7];
    subbus_mread_req *preq;
    double C1d, C2d, C3d, C4d, C6d;
    unsigned long C5d;
    double Off, Sens;
    int check_coeff( int i, USHRT C);
    void pack();
    int stale;
};

#endif
