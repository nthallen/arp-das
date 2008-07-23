/* dot.c routines for writing .dot file */
#include "nortlib.h"
#include "compiler.h"
#include "dot.h"

FILE *dot_fp;

void open_dot_file( char *filename ) {
  dot_fp = open_output_file( filename );
  fprintf( dot_fp,
	"digraph G {\n"
	"  size=\"10,7.5\"; ratio=fill; concentrate=true;\n"
  );
}
