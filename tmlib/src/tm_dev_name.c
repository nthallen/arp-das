/* nl_make_name() provides a general-purpose approach to finding other
 */
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "company.h"
#include "nortlib.h"
#include "tm.h"
#include "nl_assert.h"

char *tm_dev_name(const char *base) {
  static char name[PATH_MAX];
  char *exp;
  int nb;
  
  assert(base != NULL);
  exp = getenv("Experiment");
  if (exp != NULL) exp = "none";
  nb = snprintf( name, PATH_MAX, "/dev/%s/%s/%s", COMPANY, exp, base );
  if ( nb >= PATH_MAX ) {
    if (nl_response)
      nl_error(nl_response, "Constructed name for %s is too long", base);
    return(NULL);
  }
  return name;
}
/*
=Name tm_dev_name(): Build a standard name for QNX6 resource
=Subject TMlib
=Synopsis

#include "tm.h"
char *tm_dev_name(const char *base);

=Description

  tm_dev_name() returns a string of the form:
  /dev/huarp/exp/base where exp is the current value of the
  environment variable "Experiment" and base is the input
  argument string. This is the standard means of building
  resource names within the ARP Data Acquisition System
  architecture.

=Returns

  A pointer to a static buffer containing the expanded name.
  You must save the string if it is needed for long.

=SeeAlso

  =TMlib= functions.

=End
*/
