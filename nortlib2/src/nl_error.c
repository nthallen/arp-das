/* nl_error.c provides a default error routine for nortlib routines.
 * $Log$
 * Revision 1.2  1993/07/01  15:35:04  nort
 * Eliminated "unreferenced" via Watcom pragma
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "nortlib.h"

#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

int nl_err(int level, char *s, ...) {
  va_list arg;

  va_start(arg, s);
  nl_verror(stderr, level, s, arg);
  va_end(arg);
  return(level);
}

int (*nl_error)(unsigned int level, char *s, ...) = nl_err;
