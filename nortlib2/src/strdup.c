/* strdup is a replacement for the Lattice function of the same name.
 * $Log$
 * Revision 1.1  1993/07/01  14:07:31  nort
 * Initial revision
 *
   Written May 14, 1987
   Name changed May 21, 1987 from cpystr()
*/
#include <string.h>
#include "nortlib.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

/* t = nrtl_strdup(s);
char *t, *s;

strdup is a replacement for the Lattice function of the same name.
This one provides the same function, but uses my dynamic memory
allocators instead of malloc so it is guaranteed to return a valid
pointer or die!
-*/
char *nrtl_strdup(const char *s) {
  char *copy;

  copy = new_memory(strlen(s)+1);
  if (copy != NULL) strcpy(copy, s);
  return(copy);
}
