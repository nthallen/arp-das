/* cic.c Defines functions used by Command Interpreter Clients */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "nortlib.h"
#include "tm.h"
#include "cmdalgo.h"
#include "cis.h"

#define CMD_PREFIX_MAX 10
#define CMD_VERSION_MAX 80
static char cic_header[CMD_PREFIX_MAX] = "CIC";
static char *cis_node;
static int cis_fd = -1;
static int playback = 0;
/* cgc_forwarding is 'cmdgen client forwarding'. It is apparently
   only used when the local command server is a generic forwarder.
   This is supposed to address the ambiguity about who should quit
   when a quit or exit command is issued. In the forwarding case,
   cgc_forwarding is not cleared when a 'Quit' is issued, so although
   cmd_interact() will exit, we may not want the client to terminate.
   Instead, we clear cgc_forwarding explicitly on an 'Exit' command.
   That was the idea, anyway, but generic forwarding never got past
   the drawing board stage.
 */
int cgc_forwarding = 0;

void cic_options(int argcc, char **argvv, const char *def_prefix) {
  int c;
  extern char *opt_string;

  if (def_prefix != NULL)
	snprintf( cic_header, CMD_PREFIX_MAX, "%s", def_prefix );
  optind = 0;
  opterr = OPTIND_RESET;
  if (argcc > 0) do {
    c=getopt(argcc,argvv,opt_string);
    switch (c) {
      case 'C':
        cis_node = optarg;
        break;
      case 'h':
		snprintf( cic_header, CMD_PREFIX_MAX, "%s", optarg );
        break;
      case 'p':
        playback = 1;
        break;
      case '?':
        nl_error(3, "Unknown option -%c", optopt);
      default:
        break;
    }
  } while (c!=-1);
  opterr = 1;
}

/* cic_init() Locates the Command Interpreter Server (CIS) using
   either the default node information or the information set
   by cic_options -C <node>. Once located, if ci_version is
   non-empty, the version is queried. cic_init() uses the
   standard nl_response codes to determine whether fatal
   response is required. Returns zero on success.
*/
int cic_init(void) {
  cis_fd = tm_open_name( tm_dev_name( CMDSRVR_NAME ),
    cis_node, O_WRONLY );
  if (cis_fd < 0)
    return(1);
  
  /* If specified, verify version */
  if (ci_version[0] != '\0') {
    char vcheck[CMD_VERSION_MAX];
	int nb = snprintf( vcheck, CMD_VERSION_MAX, "[%s:V%s]\n",
	  cic_header, ci_version );
	if ( nb >= CMD_VERSION_MAX )
	  nl_error( nl_response, "Version string too long" );
	else {
	  int rb = write( cis_fd, vcheck, nb+1 );
	  if ( rb == -1 ) {
	    if (errno == EINVAL ) {
		  if (nl_response)
			nl_error(nl_response, "Incorrect Command Server Version");
		} else nl_error(nl_response,
		   "Error %d querying command server version", errno );
        return(1);
      }
	  if ( rb < nb+1 )
		nl_error( 1, "Vcheck rb = %d instead of %d", rb, nb+1 );
	}
  }

  return(0);
}

static long int ci_time = 0L;
void ci_settime( long int time ) {
  ci_time = time;
}

const char *ci_time_str( void ) {
  static char buf[11];
  int hour, min, sec;
  long int time = ci_time;

  if ( ! playback || time == 0 ) return "";
  if ( time < 0 ) return "-1: ";
  time = time % ( 24 * 3600L );
  hour = time / 3600;
  time = time % 3600;
  min = time / 60;
  sec = time % 60;
  sprintf( buf, "%02d:%02d:%02d: ", hour, min, sec );
  return buf;
}

/* ci_sendcmd() Sends a command to the CIS.
   If cmdtext==NULL, sends CMDINTERP_QUIT,0
    mode == 0 ==> CMDINTERP_SEND
    mode == 1 ==> CMDINTERP_TEST
    mode == 2 ==> CMDINTERP_SEND_QUIET
   Possible errors:
     Unable to locate CIS: Normally fatal: return 1
     CMDREP_QUIT from CIS: Reset cis_pid: return it
     CMDREP_SYNERR from CIS: Normally error: return it
     CMDREP_EXECERR from CIS: Normally warning: return it
*/
int ci_sendcmd(const char *cmdtext, int mode) {
  char *cmdopts = "";
  int clen, rv;
  char buf[CMD_MAX_COMMAND_IN+1];
  
  if (!playback && cis_fd < 0 && cic_init() != 0) return(1);
  if (cmdtext == NULL) {
    cmdopts = ":X";
    cmdtext = "";
    nl_error(-3, "Sending Quit to Server");
  } else {
    switch (mode) {
      case 1: cmdopts = ":T"; break;
      case 2: cmdopts = ":Q"; break;
      default: break;
    }
    clen = strlen(cmdtext);
    { int len = clen;

      if (len > 0 && cmdtext[len-1]=='\n') len--;
      nl_error( mode == 2 ? -4 : -3,
          "%s%*.*s", ci_time_str(), len, len, cmdtext);
    }
  }
  if (playback) return(0);
  clen = snprintf( buf, CMD_MAX_COMMAND_IN+1, "[%s%s]%s",
    cic_header, cmdopts, cmdtext );
  if ( clen > CMD_MAX_COMMAND_IN ) {
    nl_error( 2, "Command too long" );
    return CMDREP_SYNERR;
  }
  rv = write( cis_fd, buf, clen );
  if (rv == -1) {
    switch (errno) {
      case ENOENT: /* Expected quit condition */
        close(cis_fd);
        cis_fd = -1;
        return CMDREP_QUIT;
      case E2BIG:
        nl_error( 4, "Unexpected E2BIG from cis" );
      case EINVAL:
        nl_error( 2, "Syntax error from cis" );
        return 1;
      case EIO:
        nl_error( 2, "Execution error from cis" );
        return 1;
      default:
        nl_error( 3, "Unhandled error %d from cis", errno );
    }
    return(1);
  }
  return 0;
}

/*
=Name cic_options(): Command line initializations for Command Clients
=Subject Command Server and Client
=Subject Startup
=Synopsis

#include "nortlib.h"
void cic_options(int argcc, char **argvv, const char *def_prefix);

=Description

  cic_options() handles command-line initializations for Command
  Clients (i.e. processes which will be sending commands to a
  Command Server generated via CMDGEN.). It handles the 'C', 'h'
  and 'p' options. ('h' actually belongs to the msg library, but
  if it has been specified, we want to know about it.)

=Returns
  Nothing.

=SeeAlso
  =Command Server and Client= functions.

=End

=Name cic_init(): Locate Command Server
=Subject Command Server and Client
=Subject Startup
=Synopsis
#include "nortlib.h"
int cic_init(void);

=Description
cic_init() Locates the Command Interpreter Server (CIS) using
either the default node information or the information set
by cic_options -C node. Once located, if ci_version is
non-empty, the version is queried. cic_init() uses the
standard =nl_response= codes to determine whether fatal
response is required.

=Returns
Returns zero on success.

=SeeAlso
  =Command Server and Client= functions.

=End

=Name cic_query(): Get version information from Command Server
=Subject Command Server and Client
=Synopsis
#include "nortlib.h"
int cic_query(char *version);

=Description
  cic_query queries the command server for version information.
  If successful, the version of the server is written into the
  buffer pointed to by the version argument.

=Returns
  Returns zero on success.

=SeeAlso
  =Command Server and Client= functions.

=End

=Name ci_settime(): Update command client's time
=Subject Command Server and Client
=Name ci_time_str(): Retrieve algorithm time string
=Subject Command Server and Client
=Synopsis
#include "nortlib.h"
void ci_settime( long int time );
const char *ci_time_str( void );

=Description

ci_settime is an internal function used to update a command
client's time. This is used during playback of algorithms to
allow outbound commands to be logged with their TM time instead
of the current time.

ci_time_str() returns a string which is appropriate for
output of algorithm time when in playback mode. If not
in playback mode, or if ci_settime() has not yet been
called, an empty string is returned.

=Returns
  ci_settime() return nothing. ci_time_str() returns a
  string.

=SeeAlso
  =Command Server and Client= functions.

=End

=Name ci_sendcmd(): Send command to Command Server
=Subject Command Server and Client
=Synopsis
#include "nortlib.h"
#include "cmdalgo.h"

int ci_sendcmd(const char *cmdtext, int mode);

=Description

ci_sendcmd() is the basic method to send a command to a
CMDGEN-generated Command Server. cmdtext is a NUL-terminated
ASCII string. It should include any terminating newline (e.g.
"Quit\n") or the command will not actually be executed. The
string must conform to the syntax specified in the CMDGEN input
for the particular server or a syntax error will occur.<P>

If cmdtext is NULL, a syntax-independent quit request is sent to
the server.<P>

The mode argument takes on the following values:

<DL>
  <DT>CMDINTERP_SEND (0)
  <DD>Execute the command and log it to memo via msg().
  <DT>CMDINTERP_TEST (1)
  <DD>Check the command for syntax only, but do not execute it
  or log it.
  <DT>CMDINTERP_SEND_QUIET (2)
  <DD>Execute the command but do not log it to memo.
</DL>

=Returns
  Returns zero on success. If the command server cannot be
  located (and =nl_response= is set below 3) returns 1.
  Otherwise, the return value is the sum of a type code and a
  value. The possible types are CMDREP_QUIT, CMDREP_SYNERR and
  CMDREP_EXECERR. CMDREP_QUIT indicates that the command you sent
  was a quit request, which probably means the client should shut
  down also. CMDREP_SYNERR indicates there was a syntax error in
  the command you sent. The value portion of the return code is
  the offset within the command where the error occurred.
  CMDREP_EXECERR indicates that an error occurred when the server
  was executing the instructions associated with the command. The
  value portion of the return code is the error code.

=SeeAlso
  =ci_sendfcmd=(), =ci_settime=(), =ci_time_str=(), and
  =Command Server and Client= functions.

=End
*/
