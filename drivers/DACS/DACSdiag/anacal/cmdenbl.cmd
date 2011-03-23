%INTERFACE <subbus>

&command
	: CMDENBL &on_off * { set_cmdenbl( $2 ); }
	;
&on_off < int >
	: On { $0 = 1; }
	: Off { $0 = 0; }
	;
