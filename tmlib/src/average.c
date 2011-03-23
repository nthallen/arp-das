/* average.c support for new cycle programs */
#include "average.h"

char rcsid_average_c[] = "$Header$";

double Average_Value( Average_Data *data ) {
  double avg = 0.;
  if ( data->count > 0 )
	avg = data->sum / data->count;
  data->sum = 0;
  data->count = 0;
  return avg;
}

void Average_Point( Average_Data *data, double pt ) {
  data->count++;
  data->sum += pt;
}

/*
=Name Average_Value(): Return average of summed data
=Subject Average Functions
=Name Average_Point(): Added data point to sum for average
=Subject Average Functions
=Name Average_Data: Data type for =Average Functions=
=Subject Average Functions

=Synopsis
#include "average.h"

double Average_Value( Average_Data *data );
void Average_Point( Average_Data *data, double pt );

=Description

Average_Point() and Average_Value() are used together to simplify
realtime averaging. The Average_Data data structure holds the
current sum and number of data points. Average_Value() resets the
count and sum.

=Returns

Average_Value() returns the average of points previously passed
to Average_Point().

=SeeAlso

=Average Functions=.

=End
*/
