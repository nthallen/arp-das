/* nl_error.c provides a default error routine for nortlib routines.
 * $Log$
 * Revision 1.1  1992/09/02  13:26:38  nort
 * Initial revision
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "nortlib.h"

#ifdef __WATCOMC__
  #pragma off (unreferenced)
	static char rcsid[] =
	  "$Id$";
  #pragma on (unreferenced)
#endif

int nl_err(int level, char *s, ...) {
  va_list arg;

  va_start(arg, s);
  nl_verror(stderr, level, s, arg);
  va_end(arg);
  return(level);
}

int (*nl_error)(unsigned int level, char *s, ...) = nl_err;
