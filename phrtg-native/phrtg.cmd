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
	: File %w %d * { cis_turf( if_phrtg, "MLF %s %d\n", $2, $3 ); }
	;
