%{
  /* trial.cmd
   * $Id$
   * $Log$
# Revision 1.3  1992/07/15  20:37:45  nort
# Beta Release
#
   */
  #include <stdio.h>
  #define DCT_STR 3
  #define DCV_LmpAOn 0
  #define DCV_LmpBOn 2
  #define DCV_HtrOn 4
  #define IOMODE_INIT (IO_SPACE|IO_BACKSPACE)
  #define QNX_CONS
  void data(short type, short val) {
	printf("\nExecuting data command %d %d\n", type, val);
  }
%}
&start : &commands quit *
	;
&commands :
	: &commands &command
	: &commands Other * &othercmds
	: &commands Varchk * &varchks
	;
&command
	: *
	: &on_off_cmd &on_or_off * {
		data(DCT_STR, $1 + $2);
		printf("TSP = %d, vsp = %d\n", tsp, vsp);
	  }
	: Data %d (Command type) %d (Command Value) * { data($2, $3); }
	: IOMODE %d (BS=1 SP=2 AL=4 WD=8) * { iomode = $2; }
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
	: Root &command
	;
&varchks
	: &vchks quit *
	;
&vchks
	:
	: &vchks &varchk
	;
&varchk
	: d %d (Enter integer) *
		{ printf("\nInteger %d\n\n", $2); }
	: ld %ld (Enter long integer) *
		{ printf("\nLong Integer %ld\n\n", $2); }
	: x %x (Enter hex integer) *
		{ printf("\nHex Integer %X\n\n", $2); }
	: lx %lx (Enter long hex integer) *
		{ printf("\nLong Hex Integer %lX\n\n", $2); }
	: o %o (Enter Octal integer) *
		{ printf("\nOctal Integer %o\n\n", $2); }
	: lo %lo (Enter long Octal integer) *
		{ printf("\nLong Octal Integer %lo\n\n", $2); }
	: f %f (Enter Float) *
		{ printf("\nFloat %f\n\n", $2); }
	: lf %lf (Enter Double) *
		{ printf("\nDouble %lf\n\n", $2); }
	: w %w (Enter Word) *
		{ printf("\nWord \"%s\"\n\n", $2); }
	: s %s (Enter Line of text) *
		{ printf("\nLine \"%s\"\n\n", $2); }
	;
%%

int main(int argc, char **argv) {
  #ifdef QNX_CONS
	int i;
  
	for (i = 1; i < argc; i++) {
	  if (define_con(argv[i]))
		fprintf(stderr, "Error opening console %s\n", argv[i]);
	}
  #endif
  
  command();
  
  #ifdef QNX_CONS
	close_cons();
  #endif
  return(0);
}
