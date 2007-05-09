/* opnifile.c Replacement for default version.
 * $Log$
 * Revision 1.2  2001/10/10 20:42:30  nort
 * Compiled under QNX6. Still need to configure with automake
 * and autoconf.
 *
 * Revision 1.1  1994/09/15 19:45:28  nort
 * Initial revision
 *
 */
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "nortlib.h"
#include "compiler.h"

/* Use strrchr to find last /
   Use strrchr to find last . after last slash
   If no dot, append ".oui"
   search
   If not found and first char is not /, prefix oui/ and search again.

   New method: 
     Append .oui if no extension given
     Do not prefix with oui/.
     Look in local directory and PKGDATADIR only
     It would be neighborly to add a -I option to look elsewhere,
     but this hasn't been needed over the past 15 years.
 */

FILE *open_input_file(char *filename) {
  char fullname[ PATH_MAX ], partname[ PATH_MAX ];
  char *bn, *dot;
  int i;
  FILE *fp;
  
  strncpy(partname, filename, PATH_MAX);
  partname[PATH_MAX-1] = '\0'; // Make sure it's nul terminated
  bn = strrchr(partname, '/');
  if (bn == 0) bn = partname;
  dot = strrchr(bn, '.');
  if (dot == 0) {
	i = strlen(partname);
	strncpy(partname+i, ".oui", PATH_MAX-i);
	partname[PATH_MAX-1] = '\0';
  }
  fp = fopen( partname, "r" );
  if ( fp != NULL ) return fp;
  if ( partname[0] != '/' ) {
	snprintf( fullname, PATH_MAX, "%s/%s",
			  PKGDATADIR, partname );
    fp = fopen( fullname, "r" );
    if ( fp != NULL ) return fp;
  }
  return 0;
}
