%{
  #include <stdio.h>
  #define DCT_STR 3
  #define DCV_LmpAOn 0
  #define DCV_LmpBOn 2
  #define DCV_HtrOn 4
  void data(short type, short val) {
	printf("\nExecuting data command %d %d\n", type, val);
  }
%}
&start : &commands quit *
	;
&commands :
	: &commands &command
	;
&command
	: *
	: &on_off_cmd &on_or_off * { data(DCT_STR, $1 + $2); }
	: Data %d (Command type) %d (Command Value) * { data($2, $3); }
	: IOMODE %d (BS=1 SP=2 AL=4 WD=8) * { iomode = $2; }
	: Other * &othercmds
	;
&on_off_cmd <int>
	: Lamp A { $0 = DCV_LmpAOn; }
	: Lamp B { $0 = DCV_LmpBOn; }
	: Heater { $0 = DCV_HtrOn; }
	;
&on_or_off <int>
	: On { $0 = 0; }
	: Off { $0 = 1; }
	;
&othercmds
	: &ocmds *
	;
&ocmds
	:
	: &ocmds &ocmd
	;
&ocmd
	: a *
	: b *
	: c *
	;
%%

int main(void) {
  command();
  return(0);
}
