/* nl_verr.c contains nl_verror() which allows easy expansion of
 * the nl_error capabilities in many cases.
 * $Log$
 * Revision 1.1  1993/07/01  15:30:23  nort
 * Initial revision
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "nortlib.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

int nl_verror(FILE *ef, int level, const char *fmt, va_list args) {
  char *lvlmsg;

  if (level < -1 && nl_debug_level > level) return(level);
  switch (level) {
	case -1: lvlmsg = ""; break;
    case 0: lvlmsg = ""; break;
	case 1: lvlmsg = "Warning: "; break;
	case 2: lvlmsg = "Error: "; break;
	case 3: lvlmsg = "Fatal: "; break;
	default:
	  if (level <= -2) lvlmsg = "Debug: ";
	  else lvlmsg = "Internal: ";
	  break;
  }
  fprintf(ef, "%s", lvlmsg);
  vfprintf(ef, fmt, args);
  fputc('\n', ef);
  if (level > 2 || level == -1) exit(level > 0 ? level : 0);
  return(level);
}
