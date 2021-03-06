%{
  #include <string.h>
  #include <errno.h>
  #include "tm.h"

%}

%INTERFACE <phrtg>

&command
	: PhRTG &phrtg_cmd
	;
&phrtg_cmd
	: File %w %d * { if_phrtg.Turf( "MLF %s %d\n", $2, $3 ); }
        : Datum %w %s * { if_phrtg.Turf( "TR %s %s\n", $2, $3 ); }
	;
