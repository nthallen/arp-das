/* strdup is a replacement for the Lattice function of the same name.
*/
#include <string.h>
#include "nortlib.h"
char rcsid_strdup_c[] =
  "$Header$";

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
