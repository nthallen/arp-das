%{
  /* yylex.l for tmcalgo
   * $Log$
   */
  #include <stdlib.h> /* for atoi */
  #include <string.h> /* for strdup */
  #include <assert.h>
  #include <ctype.h>
  #include <unistd.h> /* for read */
  #include "nortlib.h"
  #include "y.tab.h"
  #include "yytype.h"
  #include "memlib.h"
  static char rcsid[] = "$Id$";
  
  int input_line_number = 1, ta_input_line = 0;
  
  /* ta_input() returns an input character, but checks for newlines
     and EOF also */
  int ta_input(void) {
	int c;
	
	c = input();
	if (c == '\n') {
	  if (ta_input_line <= input_line_number)
		ta_input_line = input_line_number + 1;
	  else ta_input_line++;
	} else if (c == EOF)
	  nl_error(3, "Unexpected EOF during TMC statement");
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
	if (sbuf->n_chars >= sbuf->size)
	  nl_error(4, "STATBUF overflow: arbitrary limit exceeded");
	sbuf->text[sbuf->n_chars++] = c;
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
						  buffer_char(&sbuf, d);
						  if (d == c) break;
						  else if (d == '\\') {
							d = ta_input();
							buffer_char(&sbuf, d);
						  }
						}
						break;
					}
					if (level == 0) break;
					buffer_char(&sbuf, c);
				  }
				  yylval.textval = strdup(sbuftext(&sbuf));
				  if (ta_input_line > input_line_number)
					input_line_number = ta_input_line;
				  return TK_TMCSTAT;
				}
[+{}:]			return(yytext[0]);
[ \t\f]       	;
[;%#].*			;
\n          	input_line_number++;
.           	nl_error(2, "Illegal Character");