%{
  /* yylex.l Lexical Analyzer for parser generator
   * $Log$
   * Revision 1.8  2007/04/26 15:50:44  ntallen
   * Changed the name of the include file for use with automake
   *
   * Revision 1.7  2006/05/30 00:17:09  ntallen
   * Portability (to compile under cygwin)
   *
   * Revision 1.6  2004/10/08 17:07:19  nort
   * Mostly keyword differences
   *
   * Revision 1.5  1995/05/25  17:19:05  nort
   * Use standard nortlib compiler functions
   *
   * Revision 1.4  1993/07/09  19:48:27  nort
   * *** empty log message ***
   *
   * Revision 1.3  1993/05/18  13:08:34  nort
   * More comments, client/server support
   *
   * Revision 1.2  1992/10/20  20:31:32  nort
   * *** empty log message ***
   *
   * Revision 1.1  1992/10/20  19:45:08  nort
   * Initial revision
   *
   * Revision 1.1  1992/07/09  18:36:44  nort
   * Initial revision
   *
   */
   #include <string.h>
   #include <unistd.h>
   #include "cmdgen.h"
   #include "yyparse.h"
   #include "compiler.h"
   #include "nortlib.h"
   #pragma off (unreferenced)
	 static char rcsid[] =
	   "$Id$";
   #pragma on (unreferenced)

   #ifdef yywrap
	 #undef yywrap
   #endif
   int yywrap(void);
   #define ACTBUFSIZE 2048
   #define ret_full_str(x) { yylval.str_val = strdup(yytext);\
							 return(x); }
   #define ret_inner(x) { yytext[yyleng-1] = '\0';\
						  yylval.str_val = strdup(yytext+1);\
						  return(x); }

   /* This lexical analyzer is designed to accept most english as
	  words, and hence keywords are kept to an absolute minimum.
	  Most syntactic elements are provided with punctuation.
	  The basic elements are:
			&name		non-terminal
			&&name		client-specific non-terminal
			<type>		non-terminal type specification
			%x			variable input specification
			(prompt)	prompt text (with inner parens escaped)
			{ C code }	C code with balanced braces
			%{ C code %}C code copied directly to output

	  Client-specific non-terminals are denoted internally by
	  the fact that their names begin with '&'. i.e. all
	  non-terminals are stored without one leading '&'.
   */
%}

%%

^"%{" {
  int c, d;

  c = input();
  for (;;) {
	if (c == EOF) compile_error(3, "Unexpected EOF in %{");
	fputc(c, ofile);
	if (c == '\n') {
	  input_linenumber++;
	  c = input();
	  if (c == '%') {
		d = input();
		if (d == '}') break;
		fputc(c, ofile);
		c = d;
	  }
	} else c = input();
  }
}
\{  {
  int level, c, i;
  static char act[ACTBUFSIZE];

  act[0] = '{';
  for (level = 1, i = 1; level > 0; ) {
	if (i == ACTBUFSIZE)
	  compile_error(4, "Action Buffer Arbitrary Limit Exceeded");
	act[i++] = c = input();
	if (c == '{') level++;
	else if (c == '}') level--;
	else if (c == '\n') input_linenumber++;
	else if (c == EOF)
	  compile_error(3, "Unexpected EOF reading C Code");
  }
  act[i] = '\0';
  yylval.str_val = strdup(act);
  return(TK_C_CODE);
}
\&\&?[a-z][a-z_0-9]+ {
	yylval.nt_val = non_terminal(yytext+1);
	return(TK_NON_TERMINAL);
  }
%INTERFACE {
    yylval.str_val = "";
    return( TK_INTERFACE );
  }
\%l?[dxofws] ret_full_str(TK_VAR_SPEC);
\(([^\n\)]|\\.)*\) ret_inner(TK_PROMPT);
\<([^\n\>]|\\.)*\> ret_inner(TK_TYPE_SPEC);
#.* ;
[ \t] ;
[\n] input_linenumber++;
[:;*] return(yytext[0]);
[^ {\t\n%][^ \t\n]* ret_full_str(TK_WORD);
