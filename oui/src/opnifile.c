/* opnifile.c Replacement for default version.
 * $Log$
 */
#include <stdlib.h>
#include <string.h>
#include "nortlib.h"
#include "compiler.h"

/* Use strrchr to find last /
   Use strrchr to find last . after last slash
   If no dot, append ".oui"
   search
   If not found and first char is not /, prefix oui/ and search again.
 */

FILE *open_input_file(char *filename) {
  char fullname[ PATH_MAX ], partname[ PATH_MAX ];
  char *bn, *dot, *first;
  int i;
  
  strcpy(partname, "oui/");
  i = strlen(partname);
  first = partname+i;
  strncpy(first, filename, PATH_MAX-i);
  partname[PATH_MAX-1] = '\0';
  bn = strrchr(partname, '/');
  if (bn == 0) bn = partname;
  dot = strrchr(bn, '.');
  if (dot == 0) {
	i = strlen(partname);
	strncpy(partname+i, ".oui", PATH_MAX-i);
	partname[PATH_MAX-1] = '\0';
  }
  _searchenv(first, "INCLUDE", fullname);
  if (fullname[0] == '\0' && first[0] != '/')
	_searchenv(partname, "INCLUDE", fullname);
  if (fullname[0] != '\0') return fopen(fullname, "r");
  else return 0;
}
