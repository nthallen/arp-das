%INTERFACE <getcon>
&command
	: getcon end session %w (Enter Session ID) *
	    { cis_turf(if_getcon, "%s\n", $4 ); }
	;
