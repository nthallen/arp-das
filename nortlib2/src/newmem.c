/* newmem.c contains a simple new_memory() function. It is named
 * nl_new_memory() in order not to conflict with memlib.h, but
 * nortlib.h will define new_memory to be nl_new_memory if memlib.h
 * has not been included.
 */
#include <stdlib.h>
#include "nortlib.h"
char rcsid_newmem_c[] =
  "$Header$";

void *new_memory(size_t size) {
  void *p;
  
  p = malloc(size);
  if (p == 0 && nl_response)
	nl_error(nl_response, "Memory allocation error");
  return p;
}

void nl_free_memory(void *p) {
  free(p);
}
