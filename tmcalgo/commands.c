/* commands.c Handles interface to command interpreters.
 * $Log$
 * Revision 1.2  1993/09/27  20:08:08  nort
 * Cleanup, common compiler functions
 *
 * Revision 1.1  1993/05/18  20:37:22  nort
 * Initial revision
 *
 */
#include "nortlib.h"
#include "compiler.h"
#include "cmdalgo.h"
#include "yytype.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced);

char ci_version[CMD_VERSION_MAX] = "";

static int cswarn = 0;

void check_command(const char *command) {
  int old_response, rv;
  
  if (!cswarn) {
	old_response = set_response(0);
	rv = ci_sendcmd(command, 1);
	set_response(old_response);
	if (rv != 0) switch (CMDREP_TYPE(rv)) {
	  case 0:
		compile_error(1, "Command Server not found: commands untested");
		cswarn = 1;
		break;
	  case CMDREP_TYPE(CMDREP_QUIT):
		if (command != NULL)
		  compile_error(1, "Unexpected Quit code from CIS", rv);
		break;
	  case CMDREP_TYPE(CMDREP_EXECERR):
		compile_error(4, "Unexpected Execution Error %d from CIS", rv);
	  case CMDREP_TYPE(CMDREP_SYNERR):
		compile_error(2, "Text Command Syntax Error:");
		compile_error(2, "%s", command);
		compile_error(2, "%*s", rv - CMDREP_SYNERR, "^");
		break;
	}
  }
}

void get_version(FILE *ofp) {
  int old_response, rv;

  old_response = set_response(0);
  rv = cic_query(ci_version);
  set_response(old_response);
  if (rv) {
	compile_error(1, "Command Server not found: No version info.");
  }
  fprintf(ofp, "%%{\n"
	"  #include \"nortlib.h\"\n"
	"  #include \"tma.h\"\n"
	"  char ci_version[] = \"%s\";\n"
	"%%}\n", ci_version);
}
