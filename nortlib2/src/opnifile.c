/* opnifile.c contains open_input_file() for compiler.h package.
 * Separated from compiler.c to allow customization.
 * $Log$
 */
#include <stdio.h>
#include "compiler.h"

FILE *open_input_file(char *filename) {
  return fopen(filename, "r");
}
