#include <limits.h>
#include <time.h>
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
  dT /= 100;
  for ( n=1; n < 100; n++ ) {
	maxstep = (lp[0] + dT) / n;
	minstep = DIV_UP(lp[0],n);
    recurse( npts, dT, lp, 1, minstep, maxstep,
	     n*minstep - lp[0], n, &besterr, &beststep );
  }
  return beststep;
}

RateDefP NewRateDefPtr( double samples, int navg, int specd,
                      CoordPtr pos ) {
  RateDefP newrate;
  if ( samples != 0. ) {
    long truesamples;
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

int alloc_dtoa( WaveDtoAP wdp, double delta, CoordPtr pos ) {
  if ( delta != 0 ) {
    if ( wdp->n_used >= N_DTOAS )
	  message( ERROR, "Not enough D/As in alloc_dtoa", 0, pos );
    else {
      wdp->value[wdp->n_used] = delta;
      wdp->bits[wdp->n_used++] = amps_to_bits( delta, pos );
    }
  }
  return dtoa_strs[wdp->n_used];
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

unsigned short amps_to_bits( double amps, CoordPtr pos ) {
  double bits = amps/AMPS_PER_BIT + 32768.;
  unsigned short sbits = (unsigned short) bits;
  if ( bits < MIN_DAC_BITS || bits > MAX_DAC_BITS )
    message( ERROR, "Offset DAC Value out of range", 0, pos );
  return sbits;
}

unsigned short aps_to_bits( double aps, CoordPtr pos ) {
  double bits = aps/APS_PER_BIT;
  unsigned short sbits = (unsigned short) bits;
  if ( bits < MIN_DAC_BITS || bits > MAX_DAC_BITS )
    message( ERROR, "Ramp DAC Value out of range", 0, pos );
  return sbits;
}

PTGNode RingdownPTG( double Istart, double Istop, double Istep, int ProgLen, CoordPtr pos ) {
  PTGNode PTG = PTGNULL;
  int StepCnt;
  for ( StepCnt = 0; StepCnt < ProgLen-1; StepCnt++ ) {
    double StepCrnt = Istart + StepCnt*Istep;
    PTG = PTGSeq(PTG,PTGRingData(amps_to_bits(StepCrnt,pos),StepCrnt));
  }
  PTG = PTGSeq(PTG,PTGRingData(amps_to_bits(Istop,pos),Istop));
  return PTG;
}
