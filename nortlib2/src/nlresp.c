/* nlresp.c defines nl_response.
 * nl_response defines how nortlib routines respond to error situations.
 * In most cases, an error is considered fatal, but in certain cases
 * an error can be tolerated. The nortlib routines will in general
 * support both responses based on the setting of nl_response.
 * If nl_response is NLRSP_DIE, errors result in termination
 * via nl_error(3, ...). If nl_response is NLRSP_WARN, errors
 * will result in nl_error(1, ...), but the defined error return
 * will occur. If nl_response is NLRSP_QUIET, no error message
 * will be printed, just the error return value.
 * The values chosen for NLRSP_* are arbitrarily chosen to match
 * the type codes to nl_error.
 * $Log$
 */
#include "nortlib.h"

int nl_response = NLRSP_DIE;

int set_response(int newval) {
  int oldval;
  
  oldval = nl_response;
  nl_response = newval;
  return(oldval);
}
