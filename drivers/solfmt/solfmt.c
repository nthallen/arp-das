/* solfmt.c is the main program for solfmt, which compiles soleniod format
   files into usable code.
 * $Log$
 * Revision 1.1  2011/02/21 18:26:05  ntallen
 * QNX4 version
 *
 * Revision 1.3  2006/02/16 18:13:30  nort
 * Uncommitted changes
 *
 * Revision 1.2  1993/09/28  17:17:11  nort
 * *** empty log message ***
 *
 * Revision 1.1  1992/10/25  00:29:13  nort
 * Initial revision
 *
   Written March 24, 1987
   Modified April 8, 1987 to add input/output specifications, default
        extensions, command line switches, and output.
   Modified July 1991 for QNX.
*/

#ifdef __USAGE
%C	[-v] filename [-ofile]
	Default input extension is ".sol"
	Default output extension is ".sft"
#endif

#include <stdio.h>
#include <string.h>
#include "tokens.h"
#include "solfmt.h"
#include "nortlib.h"

static char rcsid[] =
      "$Id$";

int main(int argc, char **argv) {
  int i, j, exp_ext;
  char input[40], ofile[40];
  extern int verbose;

  init_modes();
  ofile[0] = '\0';
  for (i = 1; i < argc; i++)
    if (argv[i][0] == '-') {
      if (argv[i][1] == 'v' || argv[i][1] == 'V') verbose = 1;
      else if (argv[i][1] == 'o' || argv[i][1] == 'O') {
        exp_ext = 0;
        for (j = 0; argv[i][j+2] != '\0'; j++) {
          ofile[j] = argv[i][j+2];
          if (ofile[j] == '/') exp_ext = 0;
          else if (ofile[j] == '.') exp_ext = j;
        }
        if (!exp_ext) strcpy(ofile+j, ".sft");
        else ofile[j] = '\0';
      } else nl_error(3,"%s: Undefined Switch\n",argv[0]);
    } else {
      exp_ext = 0;
      for (j = 0; argv[i][j] != '\0'; j++) {
        input[j] = argv[i][j];
        if (input[j] == '/') exp_ext = 0;
        else if (input[j] == '.') exp_ext = j;
      }
      if (exp_ext == 0) {
        strcpy(input+j, ".sol");
        exp_ext = j;
      } else input[j] = '\0';
      if (ofile[0] == '\0') {
        strcpy(ofile, input);
        strcpy(ofile + exp_ext, ".sft");
      }
      if (open_token_file(input) == 0) read_cmd();
      else nl_error(3,"%s: Cannot open file %s\n", argv[0], input);
    }
  compile();
  output(ofile);
  return(0);
}
