#ifndef QCLICOMP_H_INCLUDED
#define QCLICOMP_H_INCLUDED
#include <math.h>
#include "err.h"

extern long gcd( long, long );
extern long PickRes( long Tcyclei, long Tstepi, CoordPtr pos );

#define usecs(x) ((long)floor((x)*1e6+.5))
#define DIV_UP(x,y) (((x)+(y)-1)/(y))
#define TZI_DEFAULT 10000
#define N_STEPS_MAX 1000

#endif
