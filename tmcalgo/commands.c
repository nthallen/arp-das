/* commands.c Handles interface to command interpreters.
 * $Log$
 */
#include "nortlib.h"
#include "cmdalgo.h"
#include "yytype.h"
static char rcsid[] = "$Id$";

char ci_version[CMD_VERSION_MAX] = "";

static void cs_warn(void) {
  static int cswarn = 0;

  if (!cswarn) {
	nl_error(1, "Command Server not found: commands untested");
	cswarn = 1;
  }
}

void check_command(const char *command) {
  int old_response, rv;
  
  old_response = set_response(0);
  rv = ci_sendcmd(command, 1);
  set_response(old_response);
  if (rv != 0) switch (CMDREP_TYPE(rv)) {
	case 0:
	  cs_warn();
	  break;
	case CMDREP_TYPE(CMDREP_QUIT):
	  if (command != NULL)
		nl_error(1, "Unexpected Quit code from CIS", rv);
	case CMDREP_TYPE(CMDREP_EXECERR):
	  nl_error(4, "Unexpected Execution Error %d from CIS", rv);
	case CMDREP_TYPE(CMDREP_SYNERR):
	  nl_error(2, "Text Command Syntax Error:");
	  nl_error(2, "%s", command);
	  nl_error(2, "%*s", rv - CMDREP_SYNERR, "^");
	  break;
  }
}

void get_version(FILE *ofp) {
  int old_response, rv;
  
  old_response = set_response(0);
  rv = cic_query(ci_version);
  set_response(old_response);
  if (rv) cs_warn();
  fprintf(ofp, "%%{\n  char ci_version[] = \"%s\";\n%%}\n", ci_version);
}
