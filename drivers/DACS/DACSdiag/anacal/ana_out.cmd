&command
	: Set &ao_chan %lf (Enter voltage 0-10) * {
	    double N = $3 * 6553.6;
	    unsigned short bits;
	    if (N > 65535) N = 65535;
	    if (N < 0) N = 0.;
	    bits = (unsigned short) N;
	    sbwr( $2, bits );
	  }
	;
&ao_chan <unsigned short>
	: AO400 { $0 = 0x400; }
#	: AO402 { $0 = 0x402; }
#	: AO404 { $0 = 0x404; }
#	: AO406 { $0 = 0x406; }
#	: AO408 { $0 = 0x408; }
#	: AO40A { $0 = 0x40A; }
#	: AO40C { $0 = 0x40C; }
#	: AO40E { $0 = 0x40E; }
#	: AO410 { $0 = 0x410; }
#	: AO412 { $0 = 0x412; }
#	: AO414 { $0 = 0x414; }
#	: AO416 { $0 = 0x416; }
#	: AO418 { $0 = 0x418; }
#	: AO41A { $0 = 0x41A; }
#	: AO41C { $0 = 0x41C; }
#	: AO41E { $0 = 0x41E; }
	;
