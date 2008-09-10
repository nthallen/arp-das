%{
  /* I have to do this so cmdgen doesn't get confused */
  #define FF_CODE "TM}\n"
%}
&tm_cmd
	: Stop * { DG_turf( "TMe\n" ); }
	: Play * { DG_turf( "TM>\n" ); }
	: Faster * { DG_turf( "TM+\n" ); }
	: Slower * { DG_turf( "TM-\n" ); }
	: FF * { DG_turf( FF_CODE ); }
	;
&command
	: Telemetry * &tm_cmds *
	;
&tm_cmds
	:
	: &tm_cmds &tm_cmd
	;
