%{
  /* yylex.l for tmcalgo
   * $Log$
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
	  compile_error(3, "Unexpected EOF during TMC statement");
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
	sbuf->text[sbuf->n_chars++] = c;
	if (sbuf->n_chars >= sbuf->size) {
	  sbuf->n_chars *= 2;
	  sbuf->text = realloc(sbuf->text, sbuf->n_chars);
	  if (sbuf->text == 0)
		compile_error(4, "No memory to expand STATBUF");
	}
  }
  
  static char *sbuftext(struct statbuf *sbuf) {
	assert(sbuf != NULL && sbuf->text != NULL);
	sbuf->text[sbuf->n_chars] = '\0';
	return(sbuf->text);
  }
%}
%%
State			return(KW_STATE);
Partition		return(KW_PARTITION);
[A-Z][A-Z0-9_]*	{ yylval.textval = strdup(yytext);
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
"["				{ int level = 1, c, d;
				  static struct statbuf sbuf;

				  empty(&sbuf);
				  for (;;) {
					switch (c = ta_input()) {
					  case '[': level++; break;
					  case ']': level--; break;
					  case '\"':
					  case '\'':
						buffer_char(&sbuf, c);
						for (;;) {
						  d = ta_input();
						  if (d == c) break;
						  else {
							buffer_char(&sbuf, d);
							if (d == '\\') {
							  d = ta_input();
							  buffer_char(&sbuf, d);
							}
						  }
						}
						break;
					}
					if (level == 0) break;
					buffer_char(&sbuf, c);
				  }
				  yylval.textval = strdup(sbuftext(&sbuf));
				  if (ta_input_line > input_linenumber)
					input_linenumber = ta_input_line;
				  return TK_TMCSTAT;
				}
[+{}:]			return(yytext[0]);
[ \t\f]       	;
[;%#].*			;
\n          	input_linenumber++;
.           	compile_error(2, "Illegal Character");
