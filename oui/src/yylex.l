%{
  /* yylex.l lexical analyzer for oui
     $Log$
   */
  #include "nortlib.h"
  #include "compiler.h"
  #include "y.tab.h"
  #include "ouidefs.h"
  #pragma off (unreferenced)
	static char rcsid[] =
	  "$Id$";
  #pragma on (unreferenced)
%}
%%

OPTS?          return(KW_OPTS);
DEFS?          return(KW_DEF);
VARS?          return(KW_VAR);
INITS?         return(KW_INIT);
SYNOPSIS       return(KW_SYNOPSIS);
SORT           return(KW_SORT);
UNSORT         return(KW_UNSORT);
PACKAGE        return(KW_PACKAGE);
INCLUDE        return(KW_INCLUDE);
"#INCLUDE"     return(KW_C_INCLUDE);
PRECEED        return(KW_PRECEED);
FOLLOW         return(KW_FOLLOW);
SWITCH         return(KW_SWITCH);
ARGS?          return(KW_ARG);
">".*$         { yylval.strval = nl_strdup(yytext+1); return(TK_STRING); }
[a-zA-Z0-9/_]+ { yylval.strval = nl_strdup(yytext); return(TK_PACKAGE); }
"<".*">" |
\".*\"         { yylval.strval = nl_strdup(yytext); return(TK_INC_FILE); }
[;%].*$
[ \t\f]
\n             input_linenumber++;
