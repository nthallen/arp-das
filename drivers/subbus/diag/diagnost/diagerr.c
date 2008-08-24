/* diagerr.c
 * $Log$
 * Revision 1.1  2008/07/23 13:05:19  nort
 * Imported
 *
 * $Id$
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "diaglib.h"

void diag_error(char *fmt, ...) {
  va_list arg;
  
  va_start(arg, fmt);
  vfprintf(stderr, fmt, arg);
  va_end(arg);
  fputc('\n', stderr);
  exit(1);
}
