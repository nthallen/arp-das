%{
  /* yylex.l for tmcalgo
   * $Log$
 * Revision 1.7  1996/05/02  13:43:09  nort
 * Added keywords for "hold until valid or else"
 *
 * Revision 1.6  1996/04/19  13:50:41  nort
 * Many changes
 *
 * Revision 1.5  1996/01/24  17:49:20  nort
 * Support for arbitrarily long input tokens
 *
 * Revision 1.4  1994/10/14  16:16:21  nort
 * Removed arbitrary limitation on STATBUF size
 *
 * Revision 1.3  1994/08/29  17:42:27  nort
 * Removed memlib.h to use simple nortlib memory interface
 *
 * Revision 1.2  1993/09/27  20:05:32  nort
 * Cleanup, common compiler functions
 *
   * Revision 1.1  1993/05/18  20:37:15  nort
   * Initial revision
   */
  #include <stdlib.h> /* for atoi, realloc */
  #include <string.h> /* for strdup */
  #include <assert.h>
  #include <ctype.h>
  #include <unistd.h> /* for read */
  #include "compiler.h"
  #include "nortlib.h"
  #include "y.tab.h"
  #include "yytype.h"
  #pragma off (unreferenced)
	static char rcsid[] =
	  "$Id$";
  #pragma on (unreferenced)
  
  int ta_input_line = 0;
  
  /* ta_input() returns an input character, but checks for newlines
     and EOF also */
  int ta_input(void) {
	int c;
	
	c = input();
	if (c == '\n') {
	  if (ta_input_line <= input_linenumber)
		ta_input_line = input_linenumber + 1;
	  else ta_input_line++;
	} else if (c == EOF)
	  compile_error(3, "Unexpected EOF during TMC or C statement");
	return(c);
  }
  
  struct statbuf {
	char *text;
	int size;
	int n_chars;
  };
  #define STATBUFSIZE 1024
  
  static void empty(struct statbuf *sbuf) {
	assert(sbuf != NULL);
	if (sbuf->text == NULL) {
	  sbuf->text = new_memory(STATBUFSIZE);
	  sbuf->size = STATBUFSIZE-1;
	}
	sbuf->n_chars = 0;
  }
  
  static void buffer_char(struct statbuf *sbuf, int c) {
	assert(sbuf != NULL && sbuf->text != NULL);
	if (sbuf->n_chars >= sbuf->size) {
	  sbuf->size = sbuf->size * 2 + 1;
	  sbuf->text = realloc(sbuf->text, sbuf->size + 1);
	  if (sbuf->text == 0)
		compile_error(4, "No memory to expand STATBUF");
	}
	sbuf->text[sbuf->n_chars++] = c;
  }
  
  static char *sbuftext(struct statbuf *sbuf) {
	assert(sbuf != NULL && sbuf->text != NULL);
	sbuf->text[sbuf->n_chars] = '\0';
	return(sbuf->text);
  }

  /* qstat
       op = open character, e.g. '{'
	   cl = close character, e.g. '}'
	   store = non-zero if outer chars are to be saved
  */
  static void qstat( int op, int cl, int store ) {
	int level = 0, c, quoting = 0;
	static struct statbuf sbuf;

	empty(&sbuf);
	ta_input_line = input_linenumber;
	c = op;
	if ( store ) buffer_char(&sbuf, c);
	if ( op == '\"' ) quoting = op;
	else level++;
	c = ta_input();
	for (;;) {
	  if ( quoting ) {
		for (;;) {
		  if (c == quoting) break;
		  else {
			buffer_char(&sbuf, c);
			if (c == '\\') {
			  c = ta_input();
			  buffer_char(&sbuf, c);
			}
		  }
		  c = ta_input();
		}
		quoting = 0;
		if ( op == '\"' ) break;
		buffer_char(&sbuf, c);
		c = ta_input();
	  }
	  if ( c == op ) level++;
	  else if ( c == cl ) level--;
	  else if ( c == '\"' || c == '\'' ) quoting = c;
	  if ( level == 0 ) break;
	  buffer_char( &sbuf, c );
	  c = ta_input();
	}
	if ( store ) buffer_char( &sbuf, c );
	yylval.textval = strdup(sbuftext(&sbuf));
	input_linenumber = ta_input_line;
  }

  static void pipe_code(void) {
	int c;

    ta_input_line = input_linenumber;
	fprintf( ofile, "%%{" );
	c = ta_input();
	for (;;) {
	  fputc( c, ofile );
	  if (c == '\n') {
		c = ta_input();
		if (c == '%') {
		  c = ta_input();
		  if (c == '}') break;
		  fputc( '%', ofile );
		  fputc( c, ofile );
		}
	  } else c = ta_input();
	}
	fprintf( ofile, "%%}\n" );
	input_linenumber = ta_input_line;
  }
%}
%Start statedef
%%
State			return(KW_STATE);
Partition		return(KW_PARTITION);
depending       return(KW_DEPENDING);
on              return(KW_ON);
Validate        return(KW_VALIDATE);
valid           return(KW_VALID);
Hold            return(KW_HOLD);
until           return(KW_UNTIL);
or              return(KW_OR);
else            return(KW_ELSE);
[A-Z][A-Z0-9_]*	{ yylval.textval = strdup(yytext);
				  BEGIN statedef;
				  return(TK_NAME);
				}
[0-9]+			{ yylval.intval = atoi(yytext);
				  return(TK_INTEGER);
				}
">".*			{ int i;

				  /* ! isspace handles the end of the string also
					 since '\0' is not a space char.
				   */
				  for (i = 1; isspace(yytext[i]); i++);
				  yylval.textval = strdup(&yytext[i]);
				  return(TK_COMMAND);
				}
<statedef>[()]  { return yytext[0]; }
<statedef>"{"   { BEGIN 0; return yytext[0]; }
"("				{ qstat( '(', ')', 0 ); return TK_PARENSTAT; }
"{"				{ qstat( '{', '}', 0 ); return TK_TMCSTAT; }
"\""			{ qstat( '"', '"', 1 ); return TK_QSTRING; }
^"%{"           { pipe_code(); }
[+:;},]	    	{ BEGIN 0; return(yytext[0]); }
[ \t\f]       	;
"#".*			;
\n          	input_linenumber++;
.           	compile_error(2, "Illegal Character");
