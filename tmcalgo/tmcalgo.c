/* tmcalgo.c Main program for tmcalgo
 * $Log$
 */
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include "nortlib.h"
#include "yytype.h"
static char rcsid[] = "$Id$";

static int error_level = 0;

static int ta_err(int level, char *s, ...) {
  va_list arg;

  if (level >= -1 || level >= nl_debug_level) {
	if (input_line_number > 0)
	  fprintf(stderr, "%d - ", input_line_number);
	va_start(arg, s);
	nl_verror(stderr, level, s, arg);
	va_end(arg);
  }
  if (level > error_level) error_level = level;
  return(level);
} 

int (*nl_error)(int level, char *s, ...) = ta_err;

char *opt_string = "vh:" OPT_CIC_INIT;

int main(int argc, char **argv) {
  int c;

  optind = 0; /* start from the beginning */
  opterr = 0; /* disable default error message */
  while ((c = getopt(argc, argv, opt_string)) != -1) {
	switch (c) {
	  case 'v':
		nl_debug_level--;
		break;
	  case '?':
		nl_error(3, "Unrecognized Option -%c", optopt);
	  default:
		nl_error(4, "Unsupported Option -%c", c);
	}
  }
  cic_options(argc, argv, NULL);
  yyparse();
  if (error_level >= 2) exit(error_level);
  input_line_number = 0;
  get_version(stdout);
  list_states(stdout);
  output_states(stdout);
  return(0);
}
