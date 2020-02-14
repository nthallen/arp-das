#include <stdio.h>
#include <limits.h>
#include <time.h>
#include <stdarg.h>
#include "qclicomp.h"


/* Strategy: Calculate cumerr based on minstep. Propogate
   cumerr and a multiplier. If we have to increase minstep,
   we also have to increase cumerr by the increase in
   minstep times the multiplier. The
   multiplier is simply the sum of the values of m or n.
   
   Prune if cumerr already exceeds besterr.
   
   Add weighting penalty for going with really small
   stepsizes. This is the cost/benefit analysis.
   Cost is the number of steps (which is actually
   cummult!) and the benefit is low cumulative error.
   We want to minimize the cost/benefit ratio,
   which is to say, minimize cummult*cumerr.
*/
static int recurse( int npts, long dT, long *lp, int i,
    long minstep, long maxstep,
    long cumerr, int cummult, long *besterr, long *beststep ) {
  long min_m, max_m, m;
  long myerr = cumerr*cummult;
  if ( minstep < 2 ) minstep = 2;
  if ( maxstep >= 65536L ) maxstep = 65535L;
  if ( minstep > maxstep || (*besterr >= 0 && myerr >= *besterr ) )
    return 0;
  if ( i >= npts ) {
    /* we have a successful candidate. Now how do we optimize?
    Accept minstep: since m is fixed, increasing step will
    increase the error. */
    *besterr = myerr;
    *beststep = minstep;
    /* printf( "%ld %ld %ld\n", minstep, cumerr, myerr ); */
    return 1;
  }
  min_m = DIV_UP(lp[i],maxstep);
  max_m = (lp[i]+dT) / minstep;
  for ( m = min_m; m <= max_m; m++ ) {
    long myminstep = DIV_UP(lp[i],m);
    long mymaxstep = (lp[i]+dT) / m;
    long mycumerr = cumerr;
    if ( myminstep > minstep ) {
      mycumerr += (myminstep-minstep) * cummult;
    } else {
      myminstep = minstep;
    }
    if ( mymaxstep > maxstep ) mymaxstep = maxstep;
    mycumerr += myminstep*m - lp[i];
    recurse( npts, dT, lp, i+1, myminstep, mymaxstep,
            mycumerr, cummult+m, besterr, beststep );
  }
  return 0;
}

long PickRes( int npts, long *lp ) {
  long minstep, maxstep;
  long besterr = -1.;
  long beststep = 0;
  long dT = 0;
  int n;

  for ( n = 0; n < npts; n++ ) dT += lp[n];
  dT = (dT < 100 ) ? 1 : ( dT / 100 );
  for ( n=1; n < 100; n++ ) {
    maxstep = (lp[0] + dT) / n;
    minstep = DIV_UP(lp[0],n);
    recurse( npts, dT, lp, 1, minstep, maxstep,
         n*minstep - lp[0], n, &besterr, &beststep );
  }
  return beststep;
}

RateDefP NewRateDefPtr( double samples, int navg, int specd,
            int digitizer, CoordPtr pos ) {
  RateDefP newrate;
  if ( samples != 0. ) {
    long truesamples;
    switch ( digitizer ) {
      case DIG_CPCI14:
        if ( samples > 1e7 ) {
          truesamples = 10000000L;
          message( WARNING,
            "Target sample rate exceeds CPCI14 specs", 0, pos );
        } else if ( samples < 6.25e5 ) {
          navg *= floor(6.25e5/samples);
          truesamples = 625000L;
        } else {
          long count = 40000000L/((long)floor( samples ));
          truesamples = 40000000L/count;
        }
        if ( specd && fabs(samples-truesamples) > .5 ) {
          message( WARNING,
            "Sample rate adjusted to match hardware",
            0, pos );
        }
        samples = truesamples;
        break;
      case DIG_CS210:
        if ( samples > 1e7 ) {
          truesamples = 10000000L;
          message( WARNING,
            "Target sample rate exceeds CS210 specs", 0, pos );
        } else {
          long divisor;
          if ( samples < 153 ) {
            int my_navg = (153+samples-1)/samples;
            navg *= my_navg;
            samples *= my_navg;
          }
          divisor = 10000000L/samples;
          truesamples = 10000000L/divisor;
          if ( specd && fabs(samples-truesamples) > .5 ) {
            message( WARNING,
              "Sample rate adjusted to match hardware",
              0, pos );
          }
          samples = truesamples;
        }
        break;
      case DIG_SSP:
        if ( samples > 1e8 ) {
          truesamples = 100000000L;
          message( WARNING,
            "Target sample rate exceeds SSP specs", 0, pos );
        } else if ( samples < 1e8/32 ) {
          int scale = ceil((1e8/32)/samples);
          navg *= scale;
          samples *= scale;
        }
        { long count = 100000000L/((long)floor( samples ));
          truesamples = 100000000L/count;
        }
        if ( specd && fabs(samples-truesamples) > .5 ) {
          message( WARNING,
            "Sample rate adjusted to match hardware",
            0, pos );
        }
        samples = truesamples;
        if ( navg > 256 )
          message( DEADLY,
            "NAverage (specified or derived) exceeds SSP specifications",
            0, pos );
        break;
      default:
        message( DEADLY, "Unknown digitizer in NewRateDef", 0, pos );
    }
  }
  newrate = (RateDefP) malloc(sizeof(RateDef));
  if (newrate == 0)
    message( DEADLY, "Out of memory in NewRateDef", 0, pos );
  newrate->samples = samples;
  newrate->naverage = navg;
  return newrate;
}

longP Set_rval( longP rvals, int index, double val ) {
  rvals[index] = usecs(val);
  return rvals;
}

long round_to_step( double time, long step ) {
  long itime = usecs(time);
  long divisor = DIV_UP(itime,step);
  return step*divisor;
}

/* D/A Allocation. This approach is only used for ICOS ramps.
   The DACs are numbered 0 to 3. DAC1 is the Ramp DAC, and
   DACs 0, 2 and 3 provide offsets. I handle the ramp DAC
   separately and allocate the offset DACs dynamically here,
   so the offset DACs are effectively renumbered 0, 1, 2.
   
   In order to support the allocation of DAC3 to an external
   purpose (e.g. QCL Heater), I will effectively reduce the
   number of DACs that are available.
*/
static int n_dtoas_available = N_DTOAS;
static int dtoa_strs[N_DTOAS+1] = {
  0,
  SW_DAC0_OUT,
  SW_DAC0_OUT | SW_DAC2_OUT,
  SW_DAC0_OUT | SW_DAC2_OUT | SW_DAC3_OUT
};

WaveDtoAP new_wavedtoa( void ) {
  int i;
  WaveDtoAP wdp = (WaveDtoAP) malloc(sizeof(WaveDtoA));
  if (wdp == 0)
    message( DEADLY, "Out of memory in new_wavedtoa", 0,
                NoPosition );
  for (i = 0; i < N_DTOAS; i++)
    wdp->value[i] = 0.;
  wdp->n_used = 0;
  return wdp;
}

int alloc_dtoa( WaveDtoAP wdp, double delta, int qclicfg, CoordPtr pos ) {
  if ( delta != 0 ) {
    if ( wdp->n_used >= n_dtoas_available )
      message( ERROR, "Not enough D/As in alloc_dtoa", 0, pos );
    else {
      wdp->value[wdp->n_used] = delta;
      wdp->bits[wdp->n_used++] = amps_to_bits( delta, qclicfg, pos );
    }
  }
  return dtoa_strs[wdp->n_used];
}

static void set_dac3( WaveDtoAP wdp, int bits, double value, CoordPtr pos ) {
  char buf[80];
  if ( wdp->n_used )
    message( ERROR, "n_used > 0 in set_dac3", 0, pos );
  // fprintf( stderr, "set_dac3( %d, %f )\n", bits, value );
  // message( NOTE, buf, 0, pos );
  n_dtoas_available = N_DTOAS - 1;
  wdp->value[2] = value;
  wdp->bits[2] = (unsigned short) bits;
}

void set_dac3_bits( WaveDtoAP wdp, int bits, CoordPtr pos ) {
  double value = (bits + 749.8)/3252.5;
  set_dac3(wdp, bits, value, pos );
}

void set_dac3_value( WaveDtoAP wdp, double value, CoordPtr pos ) {
  double dbits = value * 3252.5 - 749.8;
  int ibits = (int) dbits;
  if ( dbits > 65535 || dbits < 0 )
    message( ERROR, "DAC3 value out of range", 0, pos );
  set_dac3(wdp, ibits, value, pos );
}

double dtoa_value( WaveDtoAP wdp, int index ) {
  if ( index < 1 || index > N_DTOAS )
      message( DEADLY, "Index out of range in dtoa_value", 0,
            NoPosition );
  return wdp->value[index-1];
}

short dtoa_bits( WaveDtoAP wdp, int index ) {
  if ( index < 1 || index > N_DTOAS )
      message( DEADLY, "Index out of range in dtoa_bits", 0,
            NoPosition );
  return wdp->bits[index-1];
}

void ptg_output_short( PTG_OUTPUT_FILE file, short value ) {
  char buf[10];
  fprintf( file, "0x%04X", value );
}

void ptg_output_word( PTG_OUTPUT_FILE file, unsigned short value, int count ) {
  if ( count > 1 )
    fprintf( file, "%04X x %d:", value, count );
  else
    fprintf( file, "%04X:", value );
}

void ptg_output_time( PTG_OUTPUT_FILE file ) {
  time_t value = time(NULL);
  ptg_output_word( file, (unsigned short)(value&0xFFFFL), 1);
  fprintf( file, " Time\n" );
  ptg_output_word( file, (unsigned short)((value >> 16) & 0xFFFFL), 1);
  fprintf( file, "\n" );
}

void ptg_output_name( PTG_OUTPUT_FILE file, const char *name ) {
  fprintf( file, "  \"%-10.10s\"", name );
}

/* The current configurations use the same scale for A/Bit */
static double amps_per_bit[QCLI_CFG_MAX+1] =
  { AMPS_PER_BIT, AMPS_PER_BIT/.33,
    AMPS_PER_BIT/10, AMPS_PER_BIT, AMPS_PER_BIT/25,
    AMPS_PER_BIT/2.41 };

#define MFBUFSIZE 80
void messagef( int severity, CoordPtr pos, char *fmt, ... ) {
  char buf[MFBUFSIZE];
  va_list arg;
  va_start(arg, fmt);
  vsnprintf( buf, MFBUFSIZE, fmt, arg );
  va_end(arg);
  message( severity, strdup(buf), 0, pos );
}

unsigned short amps_to_bits( double amps, int qclicfg, CoordPtr pos ) {
  double bits;
  unsigned short sbits;
  if ( qclicfg < 0 || qclicfg > QCLI_CFG_MAX )
    message( DEADLY, "QCLI Config Code out of range", 0, pos );
  bits = amps/amps_per_bit[qclicfg] + AMPS_BIT_OFFSET;
  sbits = (unsigned short) bits;
  if ( bits < MIN_DAC_BITS )
    messagef( ERROR, pos, "Offset DAC Value (%.3lf A) is below min (%.3lf A)",
      amps, (MIN_DAC_BITS-AMPS_BIT_OFFSET)*amps_per_bit[qclicfg] );
  if ( bits > MAX_DAC_BITS )
    messagef( ERROR, pos, "Offset DAC Value (%.3lf A) is above max (%.3lf A)",
      amps, (MAX_DAC_BITS-AMPS_BIT_OFFSET)*amps_per_bit[qclicfg] );
  //if ( bits < MIN_DAC_BITS || bits > MAX_DAC_BITS )
  //  message( ERROR, "Offset DAC Value out of range", 0, pos );
  return sbits;
}

static double aps_per_bit[QCLI_CFG_MAX+1] =
  { APS_PER_BIT, APS_PER_BIT/.33,
   .4*APS_PER_BIT, 4*APS_PER_BIT, 4*APS_PER_BIT/25.,
   APS_PER_BIT/2.41 };
unsigned short aps_to_bits( double aps, int qclicfg, CoordPtr pos ) {
  double bits;
  unsigned short sbits;
  if ( qclicfg < 0 || qclicfg > QCLI_CFG_MAX )
    message( DEADLY, "QCLI Config Code out of range", 0, pos );
  bits = aps/aps_per_bit[qclicfg] + APS_BIT_OFFSET;
  sbits = (unsigned short) bits;
  if ( bits < MIN_DAC_BITS )
    messagef( ERROR, pos, "Ramp DAC Value (%.1lf A/sec) %s (%.1lf A/sec)",
        aps,
        APS_PER_BIT < 0 ? "above max" : "below min",
        (MIN_DAC_BITS-APS_BIT_OFFSET)*aps_per_bit[qclicfg]);
  if ( bits > MAX_DAC_BITS )
    messagef( ERROR, pos, "Ramp DAC Value (%.1lf A/sec) %s (%.1lf A/sec)",
        aps,
        APS_PER_BIT > 0 ? "above max" : "below min",
        (MAX_DAC_BITS-APS_BIT_OFFSET)*aps_per_bit[qclicfg]);
  return sbits;
}

PTGNode RingdownPTG( double Istart, double Istop, double Istep, int ProgLen,
      int Ncoadd, int qclicfg, CoordPtr pos ) {
  PTGNode PTG = PTGNULL;
  int StepCnt;
  int coadd;
  int Nsteps = ProgLen / Ncoadd;
  if (ProgLen > 1) {
    if (ProgLen % Ncoadd != 0)
      message( DEADLY, "(ProgLen % Ncoadd) is non-zero in RingdownPTG", 0, pos );
    for ( StepCnt = 0; StepCnt < Nsteps-1; StepCnt++ ) {
      for (coadd = 0; coadd < Ncoadd; ++coadd) {
        double StepCrnt = Istart + StepCnt*Istep;
        PTG = PTGSeq(PTG,
          PTGRingData(amps_to_bits(StepCrnt,qclicfg,pos),StepCrnt));
      }
    }
    for (coadd = 0; coadd < Ncoadd; ++coadd) {
      PTG = PTGSeq(PTG,
        PTGRingData(amps_to_bits(Istop,qclicfg,pos),Istop));
    }
  } else {
    PTG = PTGSeq(PTG,
      PTGRingData(amps_to_bits(Istop,qclicfg,pos),Istop));
  }
  return PTG;
}
