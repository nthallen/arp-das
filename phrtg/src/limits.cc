/** \file limits.cc
 * Support routines for RTG_Limits class;
 */
#include "ablibs.h"
#include "phrtg.h"

RTG_Limits::RTG_Limits() {
  min = max = epoch = 0.;
  units_per_Mtick = 20.;
  limits_trend = false;
  limits_auto = true;
  limits_current = false;
  limits_empty = true;
}

/**
 * Compare the limits to the data range and change
 * the limits if necessary. Currently simply matches
 * the range, but a more sophisticated implementation
 * may look at the scale, and choose limits that work
 * well with tick labels, etc.
 * @return true if the limits have changed.
 */
bool RTG_Limits::changed( RTG_Range &R ) {
  limits_current = true;
  if ( limits_empty ) {
    if (R.range_is_empty) return false;
    min = R.min;
    max = R.max;
    limits_empty = false;
    return true;
  }
  if (R.range_is_empty) {
    limits_empty = true;
    return true;
  }
  if ( min == R.min && max == R.max )
    return false;
  min = R.min;
  max = R.max;
  return true;
}
