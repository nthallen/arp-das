%{
  /* yylex.l lexical analyzer for oui
     $Log$
     Revision 1.3  2001/10/10 20:42:32  nort
     Compiled under QNX6. Still need to configure with automake
     and autoconf.

     Revision 1.2  2001/10/05 20:44:25  nort
     Totally redone

 * Revision 1.1  1994/09/15  19:45:27  nort
 * Initial revision
 *
   */
  #include "nortlib.h"
  #include "compiler.h"
  #include "ouidefs.h"
  #include "ouigrmr.h"
  #pragma off (unreferenced)
	static char rcsid[] =
	  "$Id$";
  #pragma on (unreferenced)
  /* Strategy:
     I want most things to be handled by the line case, but
	 after reading one of the keywords which takes arguments,
	 I want to find different tokens.
	 Blank lines are ignored.
  */
%}
%Start args opts line lline
%%

^[ \t]*"<"OPTS?">"[ \t]*\"  { BEGIN opts; return(KW_OPTS); }
^[ \t]*"<"PACKAGE">"        { BEGIN args; return(KW_PACKAGE); }
^[ \t]*"<"INCLUDE">"        { BEGIN args; return(KW_INCLUDE); }
^[ \t]*"<"PRECEED">"        { BEGIN args; return(KW_PRECEED); }
^[ \t]*"<"FOLLOW">"         { BEGIN args; return(KW_FOLLOW); }
^[ \t]*"<"SYNOPSIS">"       return(KW_SYNOPSIS);
^[ \t]*"<"COMMENT">"        return(KW_COMMENT);
^[ \t]*"<"DEFS?">"          return(KW_DEF);
^[ \t]*"<"VARS?">"          return(KW_VAR);
^[ \t]*"<"INITS?">"         return(KW_INIT);
^[ \t]*"<"SORT">"           return(KW_SORT);
^[ \t]*"<"UNSORT">"         return(KW_UNSORT);
^[ \t]*"<"SWITCH">"         return(KW_SWITCH);
^[ \t]*"<"ARGS?">"          return(KW_ARG);
^[ \t]*"<"BLANK">"          {
						yylval.strval = nl_strdup("");
						return(TK_LINE);
					  }
^[ \t]*"<"[a-zA-Z0-9/_]*">" {
						compile_error(2, "Unknown keyword: %s", yytext);
					  }
^[ \t]*"<"            {
						compile_error(1, "Rejecting");
						BEGIN lline;
						REJECT;
					  }
<opts>[^\n"]*/\"      {
						yylval.strval = nl_strdup(yytext);
						input(); /* swallow closing quote */
						return(TK_STRING);
					  }
<args>[a-zA-Z0-9/_]+  {
						yylval.strval = nl_strdup(yytext);
						return(TK_PACKAGE);
					  }
<args>"<"[^\n>]*">" |
<args>\"[^\n"]*\"     {
						yylval.strval = nl_strdup(yytext);
						return(TK_INC_FILE);
					  }
[ \t\f]+
<lline>.* |
<line>[ \t]*[^ \t\n<].*  {
						yylval.strval = nl_strdup(yytext);
						return(TK_LINE);
					  }
\n                    { BEGIN line; input_linenumber++; }
.                     { compile_error(3, "Lexical Failure"); }
