%{
  #include <string.h>
  #include <errno.h>
  #include "tm.h"
  #include "msg.h"
%}

%INTERFACE <DG:DG/cmd>
# %INTERFACE <lgr>
%INTERFACE <Quit>

&start
	: &commands Quit * %BLOCK_KB { msg(0,"Shutting down"); }
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
	: Start * { if_DG.Turf( "TMc\n" ); }
	: Single Step * { if_DG.Turf( "TMs\n" ); }
#	: Logging Suspend * { if_lgr.Turf( "TM Logging Suspend\n" ); }
#	: Logging Resume * { if_lgr.Turf( "TM Logging Resume\n" ); }
	;
