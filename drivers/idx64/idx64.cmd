%INTERFACE <idx64>

%{
  #include "idx64.h"
  #ifdef SERVER
    void idx64_cmd2( const char *cmd, int drive, step_t steps) {
      if_idx64.Turf("%s%d:%u\n", cmd, drive, steps);
    }
    void idx64_cmd1( const char *cmd, int drive) {
      if_idx64.Turf("%s%d\n", cmd, drive);
    }
  #endif
%}

&command
	: &indexer_cmd
	;
&indexer_cmd
	: Drive &drive &direction &steps * {
	    if_idx64.Turf("D%c%d:%u\n", $3, $2, $4);
	  }
	: Scan &drive &direction &steps by %d (Enter Steps per Step) *
	  { if_idx64.Turf("S%c%d:%u:%u\n", $3, $2, $4, $6 ); }
	: Stop &drive * { idx64_cmd1( "SP", $2); }
	: Drive &drive Online * { idx64_cmd1("ON", $2); }
	: Drive &drive Offline * { idx64_cmd1("OF", $2); } 
	: Drive &drive Altline * { idx64_cmd1("AL", $2); } 
	: Preset &drive Position to &steps * { idx64_cmd2("PR", $2, $5); }
	: Set &drive Online Position %d (Enter Online Position) *
		{ idx64_cmd2("OP", $2, $5); }
	: Set &drive Online Delta
		%d (Enter positive number of steps between dithered online positions) *
		  { idx64_cmd2("OD", $2, $5); }
	: Set &drive Offline Delta
		%d (Enter signed number of steps from Online to Offline position) *
		  { idx64_cmd2("FD", $2, $5); }
	: Set &drive Offline Position %d (Enter Offline Position) *
		{ idx64_cmd2("FP", $2, $5); }
	: Set &drive Altline Delta
		%d (Enter signed number of steps from Online to Altline position) *
		  { idx64_cmd2("AD", $2, $5); }
	: Set &drive Altline Position %d (Enter Altline Position) *
		{ idx64_cmd2("AP", $2, $5); }
	: Set &drive Hysteresis %d (Enter Hysteresis Amount) *
		{ idx64_cmd2("HY", $2, $4); }
	: Set &drive Speed &ix_rate Hz * { idx64_cmd2("SS",  $2, $4<<8 ); }
	: Move &drive Online Position Out * { idx64_cmd1("MO", $2); }
	: Move &drive Online Position In * { idx64_cmd1("MI", $2); }
	;
&drive <byte_t>
	: Channel %d (Enter Channel Number from 0-?) { $0 = $2; }
	;
&direction <char>
	: In { $0 = 'I'; }
	: Out { $0 = 'O'; }
	: To { $0 = 'T'; }
	;
&steps <step_t>
	: %d (Enter Number of Steps or Step Number) { $0 = $1; }
	;
&ix_rate < int >
	: 53 { $0 = 0; }
	: 80 { $0 = 1; }
	: 107 { $0 = 2; }
	: 160 { $0 = 3; }
	: 267 { $0 = 4; }
	: 400 { $0 = 5; }
	: 533 { $0 = 6; }
	: 800 { $0 = 7; }
	: 1067 { $0 = 8; }
	: 1600 { $0 = 9; }
	: 2133 { $0 = 10; }
	: 3200 { $0 = 11; }
	: 5333 { $0 = 12; }
	: 8000 { $0 = 13; }
	: 10667 { $0 = 14; }
	: 16000 { $0 = 15; }
	;
