#ifndef QCLICOMP_H_INCLUDED
#define QCLICOMP_H_INCLUDED
#include <math.h>
#include "err.h"

typedef struct {
  double samples;
  int naverage;
} RateDef, *RateDefP;

extern RateDefP NewRateDefPtr( double samples, int navg,
  int specd, CoordPtr pos );

typedef long *longP;
extern longP Set_rval( longP rvals, int index, double val );
extern long PickRes( int npts, longP lp );

extern long round_to_step( double time, long step );
#define NULLRATE() NewRateDefPtr(0.,1,0,NoPosition)
#define PICKRATE(x,y) (x->samples>0?x:y)
#define usecs(x) ((long)floor((x)*1e6+.5))
#define DIV_UP(x,y) (((x)+(y)-1)/(y))
#define TZI_DEFAULT 10000
#define N_STEPS_MAX 1000

#endif
