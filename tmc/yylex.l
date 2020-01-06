%{
  /* yylex.l lexical analyzer for TMC
     -+/,()[\]{=}:;?  These characters left for individual interpretation

     Revision 1.2  2008/07/03 18:18:48  ntallen
     To compile under QNX6 with minor blind adaptations to changes between
     dbr.h and tm.h

     Revision 1.1  2008/07/03 15:11:07  ntallen
     Copied from QNX4 version V1R9

     Revision 1.10  2004/10/08 17:35:17  nort
     Uncommitted changes

     Revision 1.9  2001/03/14 15:30:05  nort
     ICONVERT?

   * Revision 1.8  1996/04/18  19:08:05  nort
   * Changes to better control frame dimensions:
   * TM MINCOLS=<n>; and explicit specification of Synch rate.
   *
   * Revision 1.7  1995/10/18  01:58:37  nort
   * *** empty log message ***
   *
   * Revision 1.6  1993/09/27  19:34:11  nort
   * Changes to use common compiler functions
   *
   * Revision 1.5  1993/05/21  19:43:32  nort
   * Added State KW
  */
  #include <string.h>
  #include <assert.h>
  #include <unistd.h>
  #include <ctype.h>
  #include <malloc.h>
  #include "nortlib.h"
  #include "rational.h"
  #include "tmcstr.h"
  #include "calibr.h"
  #include "yytype.h"
  #include "y.tab.h"
  #include "tmc.h"

  #define STORE store_text(yytext)
  #define NOSTORE store_text(NULL)
  #define ret_tok(x) { STORE; copy_text(); return(x); }
  #define ret_ptxt(x) { NOSTORE; copy_text(); return(x); }
  #define ret_notxt(x) { discard_text(); return(x); }

  static void lex_space( int n );
  static void store_text(const char *s);
  #if STORE_COMMENTS
    static void store_char(char c);
  #endif
  static void copy_text(void);
  static void discard_text(void);
  static void swallow_comment(void);

%}
%%
ADDRESS ret_notxt(KW_ADDRESS);
CALIBRATION ret_notxt(KW_CALIBRATION);
CASE ret_tok(KW_CASE);
CHAR ret_tok(KW_CHAR);
COLLECT ret_ptxt(KW_COLLECT);
CONVERT ret_notxt(KW_CONVERT);
ICONVERT ret_notxt(KW_ICONVERT);
DEFAULT ret_tok(KW_DEFAULT);
DEPENDING ret_ptxt(KW_DEPENDING);
DISPLAY ret_ptxt(KW_DISPLAY);
DO ret_tok(KW_DO);
DOUBLE ret_tok(KW_DOUBLE);
ELSE ret_tok(KW_ELSE);
FLOAT ret_tok(KW_FLOAT);
FOR ret_tok(KW_FOR);
GROUP ret_notxt(KW_GROUP);
HZ ret_notxt(KW_HZ);
IF ret_tok(KW_IF);
INT ret_tok(KW_INT);
INT8_T ret_tok(KW_INT8_T);
INT16_T ret_tok(KW_INT16_T);
INT32_T ret_tok(KW_INT32_T);
INT64_T ret_tok(KW_INT64_T);
INITFUNC ret_tok(KW_INITFUNC);
INVALIDATE ret_notxt(KW_INVALIDATE);
LONG ret_tok(KW_LONG);
MAXCOLS ret_notxt(KW_MAXCOLS);
MINCOLS ret_notxt(KW_MINCOLS);
ON ret_notxt(KW_ON);
ONCE ret_notxt(KW_ONCE);
REDRAWFUNC ret_tok(KW_REDRAWFUNC);
RETURN ret_tok(KW_RETURN);
SHORT ret_tok(KW_SHORT);
SIGNED ret_tok(KW_SIGNED);
STATE ret_tok(KW_STATE);
STRUCT ret_tok(KW_STRUCT);
SWITCH ret_tok(KW_SWITCH);
SYNCHRONOUS ret_notxt(KW_SYNCHRONOUS);
TEXT ret_notxt(KW_TEXT);
TOLERANCE ret_notxt(KW_TOLERANCE);
TM ret_ptxt(KW_TM);
TYPEDEF ret_tok(KW_TYPEDEF);
UINT8_T ret_tok(KW_UINT8_T);
UINT16_T ret_tok(KW_UINT16_T);
UINT32_T ret_tok(KW_UINT32_T);
UINT64_T ret_tok(KW_UINT64_T);
UNION ret_tok(KW_UNION);
UNSIGNED ret_tok(KW_UNSIGNED);
VALIDATE ret_notxt(KW_VALIDATE);
WHILE ret_tok(KW_WHILE);
HOURS? ret_notxt(UN_HOUR);
MINUTES? ret_notxt(UN_MINUTE);
SAMPLES? ret_notxt(UN_SAMPLE);
SECONDS? ret_notxt(UN_SECOND);
'(\\.)|[^']' ret_tok(TK_CHAR_CONST);
(->) ret_tok(TK_DEREF);
[A-Z][A-Z0-9_]* {
  struct nm *ref;

  /* return totally separate pretext and toktext values */
  NOSTORE;
  copy_text();
  yylval.l.toktext = strdup(yytext);
  ref = find_name(yytext, 0);
  if (ref != NULL && ref->type == NMTYPE_TMTYPE)
    return(TK_TYPE_NAME);
  else return(TK_NAME);
}
0x[A-F0-9]+ {
  yylval.l.intval = strtol(yytext+2,NULL,16);
  ret_tok(TK_INTEGER_CONST);
}
0[0-7]* {
  yylval.l.intval = strtol(yytext,NULL,8);
  ret_tok(TK_INTEGER_CONST);
}
[1-9][0-9]* {
  yylval.l.intval = strtol(yytext,NULL,10);
  ret_tok(TK_INTEGER_CONST);
}
\.[0-9]+([eE][-+]?[0-9]+)* |
[0-9]+\.[0-9]*([eE][-+]?[0-9]+)* ret_tok(TK_REAL_CONST);
\"([^\\"\n]|\\.)*\" ret_tok(TK_STRING_LITERAL);
"/*" swallow_comment();
"..." |
"<<=" |
">>=" |
"!=" |
"%=" |
"&&" |
"&=" |
"*=" |
"++" |
"+=" |
"--" |
"-=" |
"/=" |
"<<" |
"<=" |
"==" |
">=" |
">>" |
"^=" |
"|=" |
"||" |
[!~%&*<>^|]  ret_tok(TK_OPER_PUNC);
[ \t\f] STORE;
\n { STORE; input_linenumber++;}
^"%{" {
  int c, d;

  NOSTORE;
  c = input();
  for (;;) {
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
. if (isgraph(yytext[0])) ret_tok(yytext[0]);
%%
#define STORE_COMMENTS 0
#define LEXBUFSIZE 1024
static char *lexbuf = NULL;
static unsigned int lbidx = 0, lbsiz = 0, lbpidx = 0;

static void lex_space( int n ) {
  if (lexbuf == NULL) {
    lbsiz = LEXBUFSIZE;
    lexbuf = new_memory(lbsiz);
  }
  if (lbidx + n >= lbsiz) {
    lbsiz *= 2;
    lexbuf = realloc(lexbuf, lbsiz);
    if ( lexbuf == 0 )
      compile_error(3, "Lexbuf overflow");
  }
}
static void store_text(const char *s) {
  unsigned int len;

  len = ( s != NULL ) ? strlen(s) : 0;
  lex_space(len);
  if ( len ) strcpy(lexbuf+lbidx, s);
  lbpidx = lbidx;
  lbidx += len;
}
#if STORE_COMMENTS
  static void store_char(char c) {
    lex_space( 1 );
    lexbuf[lbidx++] = c;
  }
#endif
static void copy_text(void) {
  if (lbidx > 0) {
    lexbuf[lbidx] = '\0';
    yylval.l.pretext = strdup(lexbuf);
    yylval.l.toktext = yylval.l.pretext + lbpidx;
  } else yylval.l.pretext = yylval.l.toktext = NULL;
  lbidx = lbpidx = 0;
}
static void discard_text(void) {
  lbidx = lbpidx = 0;
  yylval.l.pretext = yylval.l.toktext = NULL;
}

/* The following function is used for swallowing comments. This
   will have to be modified when we wish to save comments. */
#if STORE_COMMENTS
  #define STORE_COMMENT STORE
  #define STORE_CHAR(x) store_char(x)
#else
  #define STORE_COMMENT
  #define STORE_CHAR(x)
#endif
static void swallow_comment(void) {
  int c, d;

  STORE_COMMENT;
  c = input();
  STORE_CHAR(c);
  for (;;) {
    if (c == '\n') input_linenumber++;
    d = input();
    STORE_CHAR(d);
    if (c == '*' && d == '/') break;
    if (c == '/' && d == '*') compile_error(2, "Comments cannot be nested");
    c = d;
  }
}
