#include <limits.h>
#include "qclicomp.h"

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
