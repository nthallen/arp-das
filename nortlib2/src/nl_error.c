/* nl_error.c provides a default error routine for nortlib routines.
 * $Log$
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "nortlib.h"

static char rcsid[] = "$Id$";

int nl_err(unsigned int level, char *s, ...) {
  char *lvlmsg;
  va_list arg;
  
  va_start(arg, s);
  switch (level) {
    case 0: lvlmsg = "Info"; break;
	case 1: lvlmsg = "Warning"; break;
	case 2: lvlmsg = "Error"; break;
	case 3: lvlmsg = "Fatal"; break;
	default: lvlmsg = "Internal"; break;
  }
  fprintf(stderr, "%s: ", lvlmsg);
  vfprintf(stderr, s, arg);
  va_end(arg);
  fputc('\n', stderr);
  if (level > 2) exit(level);
  return(level);
}

int (*nl_error)(unsigned int level, char *s, ...) = nl_err;
