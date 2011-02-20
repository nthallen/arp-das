%INTERFACE <getcon>
&command
	: getcon end session %w (Enter Session ID) *
	    { if_getcon.Turf("%s\n", $4 ); }
	;
