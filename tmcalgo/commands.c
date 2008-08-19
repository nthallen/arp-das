/* commands.c Handles interface to command interpreters.
 */
#include <string.h>
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
  if (!cswarn) {
	int rv;

	{ int old_response = set_response(0);
	  if ( command == 0 ) rv = ci_sendcmd( command, 1 );
	  else {
		char *cmdnl;
		if ( *command == '_' ) command++;
		cmdnl = new_memory( strlen( command ) + 2 );
		sprintf( cmdnl, "%s\n", command );
		rv = ci_sendcmd( cmdnl, 1);
		free_memory( cmdnl );
	  }
	  set_response(old_response);
	}
	switch (CMDREP_TYPE(rv)) {
	  case 0:
		if ( rv != 0 ) {
		  compile_error(1, "Command Server not found: commands untested");
		  cswarn = 1;
		}
		break;
	  case CMDREP_TYPE(CMDREP_QUIT):
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

void set_version(char *ver) {
  strncpy(ci_version, ver, CMD_VERSION_MAX);
  ci_version[CMD_VERSION_MAX-1] = '\0';
}

void get_version(FILE *ofp) {
  #ifndef __QNXNTO__
    int old_response, rv;

    old_response = set_response(0);
    rv = cic_query(ci_version);
    set_response(old_response);
    if (rv) {
	  compile_error(1, "Command Server not found: %s",
	    ci_version[0] ? "Using specified version." :
	    "No version info.");
    }
  #endif
  fprintf(ofp, "%%{\n"
	"  #include \"nortlib.h\"\n"
	"  #include \"tma.h\"\n"
	"  char ci_version[] = \"%s\";\n"
	"%%}\n", ci_version);
}
