/* tmcalgo.c Main program for tmcalgo
 * $Log$
 * Revision 1.1  1993/05/18  20:37:19  nort
 * Initial revision
 */
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include "nortlib.h"
#include "compiler.h"
#include "yytype.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

int (*nl_error)(int level, char *s, ...) = compile_error;

#ifdef __USAGE
%C	[options] [files]
	-k             Keep output file (.c) even on error
	-o <filename>  Send .tmc output to named file
	-v             Produce Debug Messages
	-q             Show this help message
	-w             Give error return on warnings
	-C <node>      Look for command server on specified node
#endif

char *opt_string = "h:" OPT_CIC_INIT OPT_COMPILER_INIT;

static void algo_exit(void) {
  check_command(NULL); /* Ask the server to quit */
}

int main(int argc, char **argv) {
  compile_init_options(argc, argv, ".tmc");
  cic_options(argc, argv, NULL);
  atexit(algo_exit);
  yyparse();
  if (error_level >= 2) exit(error_level);
  get_version(ofile);
  list_states(ofile);
  output_states(ofile);
  return(error_level);
}
