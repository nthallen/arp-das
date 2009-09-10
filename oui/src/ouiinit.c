/* OUI output from the following packages:
   oui
   output_ext_init
   compiler
   nl_error_init
*/
char *opt_string = "uqkwvo:";
#include "oui.h"
#include "ouidefs.h"
#include <unistd.h>
#include "nortlib.h"
#include <stdio.h>
#include "compiler.h"
  int (*nl_error)(int level, const char *s, ...) = compile_error;

void oui_init_options(int argc, char **argv) {
  char *output_extension;

  { int optltr;

	optind = OPTIND_RESET;
	opterr = 0;
	while ((optltr = getopt(argc, argv, opt_string)) != -1) {
	  switch (optltr) {
		  case 'u': sort_output = 0; break;
		  case '?':
			nl_error(3, "Unrecognized Option -%c", optopt);
		  default:
			break;
	  }
	}
  }
  output_extension = ".c";
  compile_init_options(argc, argv, output_extension);
}

#ifdef __USAGE
%C	[options] file [file ...]
	-k Keep incomplete output file on error
	-o <filename> Specify Output Filename
	-q Print usage message
	-u Do not sort the "sort" strings
	-v Increasing level of verbosity
	-w Treat warnings as errors
#endif
