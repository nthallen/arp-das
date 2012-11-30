#ifndef RINGDOWN_H_INCLUDED
#define RINGDOWN_H_INCLUDED

#include "ssp_ad.h"

typedef struct {
  double tau; // usecs
  double dtau; // usecs
  double b;
  double a;
} Ringdown_t;

extern void ringdown_setup( int RD_n_skip, int RD_n_off );
extern Ringdown_t *ringdown_fit( ssp_scan_header_t *hdr, float *data );

#endif
