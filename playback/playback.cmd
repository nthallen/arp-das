%{
  /* I have to do this so cmdgen doesn't get confused */
  #define FF_CODE "TM}\n"
%}
&tm_cmd
	: Stop * { if_DG.Turf( "TMe\n" ); }
	: Play * { if_DG.Turf( "TM>\n" ); }
	: Faster * { if_DG.Turf( "TM+\n" ); }
	: Slower * { if_DG.Turf( "TM-\n" ); }
	: FF * { if_DG.Turf( FF_CODE ); }
	;
&command
	: Telemetry * &tm_cmds *
	;
&tm_cmds
	:
	: &tm_cmds &tm_cmd
	;
