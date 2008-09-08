%{
  #include <string.h>
  #include <errno.h>
  #include "tm.h"
  static int DG_fd = -1;

  void DG_turf( char *cmd ) {
    int len, nb;
    if (DG_fd == -1 ) {
      DG_fd = open(tm_dev_name("DG/cmd"), O_WRONLY);
      if ( DG_fd == -1 ) {
      	nl_error( 2, "Unable to open DG/cmd" );
      	return;
      }
    }
    len = strlen(cmd);
    nb = write( DG_fd, cmd, len );
    if ( nb == -1 ) {
      nl_error( 2, "Error %d from DG/cmd", errno );
      close(DG_fd);
      DG_fd = -1;
    } else if (nb != len) {
      nl_error( 2, "write returned %d, expected %d", nb, len );
    } else if (nb == 0) {
      close(DG_fd);
      DG_fd = -1;
    }
  }
  #ifndef ON_QUIT_CMD
    #define ON_QUIT_CMD
  #endif

%}

# %INTERFACE <lgr>
%INTERFACE <Quit>

&start
	: &commands Quit * { DG_turf( "" ); ON_QUIT_CMD }
	: &commands &&Exit
	;
&&Exit
	: Exit * { cgc_forwarding = 0; }
	;
&commands
	:
	: &commands &command
	;
&command
	: *
	: Telemetry &tm_cmd
	: Log %s ( Enter String to Log to Memo ) * {}
	;
&tm_cmd
	: Start * { DG_turf( "TMc\n" ); }
	: Single Step * { DG_turf( "TMs\n" ); }
#	: Logging Suspend * { cis_turf( if_lgr, "TM Logging Suspend\n" ); }
#	: Logging Resume * { cis_turf( if_lgr, "TM Logging Resume\n" ); }
	;
