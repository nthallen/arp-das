#include <limits.h>
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
int recurse( int npts, long dT, long *lp, int i, long minstep, long maxstep,
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

#ifdef OLDAPPROACH
  /* PickRes is charged with determining the appropriate
	 stepsize given the best initial guess. The initial guess
	 will work as long as the following criteria are met:
	   Nsteps <= NStepsMax < 65536L
	   Tstepi < 65536L
	 
	 If the first condition is not met, the workaround is to
	 pick a larger stepsize, probably near Tcyclei/NStepsMax.
   
	 If the second condition is not met, the workaround is to
	 look for a smaller stepsize, ideally a divisor of Tstepi,
	 which is below the limit.
   
	 If both conditions fail, it's hopeless.
  */
  long PickRes( long Tcyclei, long Tstepi, CoordPtr pos ) {
	long Nsteps = Tcyclei/Tstepi;
	if ( Tstepi > USHRT_MAX ) {
	  int mindiv = ( Tstepi + USHRT_MAX ) / (USHRT_MAX + 1L);
	  int maxdiv = N_STEPS_MAX/Nsteps;
	  int div;
	  for ( div = mindiv; div <= maxdiv; div++ ) {
		if ( (Tstepi % div) == 0 ) {
		  message( NOTE, "Stepsize divided down", 0, pos );
		  return Tstepi/div;
		}
	  }
	  if ( mindiv <= maxdiv ) {
		message( WARNING,
		  "No ideal stepsize for waveform: using N_STEPS_MAX",
		  0, pos );
		return ((Tcyclei+N_STEPS_MAX-1)/N_STEPS_MAX);
	  } else {
		message( ERROR, "Stepsize for waveform exceeds 65535 usecs", 0, pos );
		return Tstepi;
	  }
	}
	if ( Nsteps > N_STEPS_MAX ) {
	  Tstepi = (Tcyclei+N_STEPS_MAX-1)/N_STEPS_MAX;
	  if ( Tstepi > USHRT_MAX )
		message( ERROR, "Waveform too long", 0, pos );
	  else
		message( WARNING,
		  "Ideal stepsize too small: using N_STEPS_MAX",
		  0, pos );
	}
	return Tstepi;
  }
#endif

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
