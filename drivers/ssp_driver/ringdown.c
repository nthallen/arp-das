#include <stdio.h>
#include <math.h>
#include "ringdown.h"

static int Fit_Skip, Fit_Offset;

void ringdown_setup( int RD_n_skip, int RD_n_off ) {
  Fit_Skip = RD_n_skip;
  Fit_Offset = RD_n_off;
}

/* returns the minimal root and optionally the maximal root */
static double quad_roots( double a, double b, double c, double *minroot ) {
  double det;
  double maxroot;
  int signum = a > 0 ? 1 : -1;
  
  det = signum * sqrt( b * b - 4 * a * c );
  if (minroot != 0) *minroot = ( -b - det ) / (2*a);
  maxroot = ( -b + det ) / (2*a);
  return maxroot;
}

/* fit_ringdown fits the data to an exponential decay, storing b,
   b-, b+, s2, a, n in buffer->this_rpt[0-5] respectively. If/when two
   channels of ringdown are supported, this routine could be
   replaced with a thin cover that passed in a pointer into
   this_rpt and modified the offset of data.
   NSample is extracted from crnt_setup.
*/
Ringdown_t *ringdown_fit( ssp_scan_header_t *hdr, float *data ) {
  static Ringdown_t rd_res;
  float *x, *y;
  double sx, sy, sxy, sx2, sy2, meanx, meany;
  double a, b, s2, bminus, bplus;
  int n = Fit_Offset;
  int N = hdr->NSamples - n - Fit_Skip;
  int i;

  sx = sy = sxy = sx2 = sy2 = 0.;
  y = data + Fit_Skip;
  x = y + n;
  for ( i = 0; i < N; ++x, ++y, ++i ) {
    sx += *x;
    sy += *y;
  }
  meanx = sx/N;
  meany = sy/N;
  y = data + Fit_Skip;
  x = y + n;
  for ( i = 0; i < N; ++x, ++y, ++i ) {
    double xd = *x - meanx;
    double yd = *y - meany;
    
    sxy += xd * yd;
    sx2 += xd * xd;
    sy2 += yd * yd;
  }

  b = quad_roots( sxy, sx2-sy2, -sxy, NULL );
  a = meany - b*meanx;

  s2 = (sy2 - 2*b*sxy + b*b*sx2) / ((N-2)*(1+b*b));

  bplus = quad_roots(
        sx2 - (N-1)*s2,
        -2*sxy,
        sy2 - (N-1)*s2,
        &bminus );
  rd_res.b = b;
  rd_res.a = a;
  rd_res.tau = n * hdr->NF * (hdr->NAvg+1) * .01 / log(b);
  rd_res.dtau = rd_res.tau - ( n * hdr->NF * .01 / log(bplus) );
  return &rd_res;
}

