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
    if ( limits_trend ) {
      epoch = R.max + R.epoch;
      max = 0.;
      min = -span; // But where is span set?
    } else {
      min = R.min + R.epoch;
      max = R.max + R.epoch;
      epoch = 0.;
    }
    limits_empty = false;
    return true;
  }
  if (R.range_is_empty) {
    limits_empty = true;
    return true;
  }
  // Neither limits nor range are empty
  if (limits_trend) {
    if ( R.max + R.epoch > epoch ) {
      epoch = R.max + R.epoch;
      return true;
    }
  } else if ( min == R.min + R.epoch && max == R.max + R.epoch )
    return false;
  min = R.min + R.epoch;
  max = R.max + R.epoch;
  return true;
}
