/* opnifile.c contains open_input_file() for compiler.h package.
 * Separated from compiler.c to allow customization.
 */
#include <stdio.h>
#include "compiler.h"
char rcsid_opnifile_c[] =
  "$Header$";

FILE *open_input_file(char *filename) {
  return fopen(filename, "r");
}
/*
=Name open_input_file(): Open compiler input file
=Subject Compiler
=Synopsis

#include <stdio.h>
#include "compiler.h"
FILE *open_input_file(char *filename);

=Description

  This is a thin cover for fopen(), but it may be selectively
  replaced if a particular compiler requires a more complex
  search, for example.

=Returns

  The FILE pointer for the specified input file.

=SeeAlso

  =Compiler= functions.

=End
*/
