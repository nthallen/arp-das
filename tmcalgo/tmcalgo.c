/* tmcalgo.c Main program for tmcalgo
 * $Log$
 * Revision 1.2  1993/09/27  20:06:04  nort
 * Cleanup, common compiler functions.
 *
 * Revision 1.1  1993/05/18  20:37:19  nort
 * Initial revision
 */
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include "nortlib.h"
#include "compiler.h"
#include "yytype.h"
#include "oui.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

static void algo_exit(void) {
  check_command(NULL); /* Ask the server to quit */
}

int main(int argc, char **argv) {
  oui_init_options( argc, argv );
  atexit(algo_exit);
  yyparse();
  if (error_level >= 2) exit(error_level);
  get_version(ofile);
  list_states(ofile); /* identifies all substates */
  output_states(ofile);
  output_mainloop(ofile);
  return(error_level);
}
