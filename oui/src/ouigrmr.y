%{
  /* $Log$
 * Revision 1.1  1994/09/15  19:45:23  nort
 * Initial revision
 *
   */
  #include "nortlib.h"
  #include "compiler.h"
  #include "ouidefs.h"
  #pragma off (unreferenced)
	static char rcsid[] =
	  "$Id$";
  #pragma on (unreferenced)

  #define yyerror(x) compile_error(2, x)
%}
%token KW_OPTS
%token KW_VAR
%token KW_INIT
%token KW_COMMENT
%token KW_DEF
%token KW_SYNOPSIS
%token KW_SORT
%token KW_UNSORT
%token KW_PACKAGE
%token KW_INCLUDE
%token KW_PRECEED
%token KW_FOLLOW
%token KW_SWITCH
%token KW_ARG
%token <strval> TK_STRING
%token <strval> TK_PACKAGE
%token <strval> TK_INC_FILE
%token <strval> TK_LINE
%type <pkgval> package
%%
starter : program
	;
program :
	| program package_def
	;
package_def : KW_PACKAGE package { oui_defpkg($2); }
	| package_def statement
	;
statement : mi_statement
	| comment
	| defs
	| vars
	| inits
	| unsort
	| sort
	| switch
	| arg
	| KW_SYNOPSIS TK_LINE { oui_synopsis($2); }
	;
mi_statement : KW_OPTS TK_STRING { oui_opts($2); }
	| include
	| preceed
	| follow
	;
comment : KW_COMMENT
	| comment TK_LINE { free_memory($2); }
	;
defs : KW_DEF
	| defs mi_statement
	| defs TK_LINE { oui_defs($2); }
	;
vars : KW_VAR
	| vars mi_statement
	| vars TK_LINE { oui_vars($2); }
	;
inits : KW_INIT
	| inits mi_statement
	| inits TK_LINE { oui_inits($2); }
	;
unsort : KW_UNSORT
	| unsort mi_statement
	| unsort TK_LINE { oui_unsort($2); }
	;
sort : KW_SORT
	| sort mi_statement
	| sort TK_LINE { oui_sort($2); }
	;
switch : KW_SWITCH
	| switch mi_statement
	| switch TK_LINE { oui_switch($2); }
	;
arg : KW_ARG
	| arg mi_statement
	| arg TK_LINE { oui_arg($2); }
	;

include : KW_INCLUDE inc_file
	| include inc_file
	;
inc_file : TK_PACKAGE { oui_include($1); }
	| TK_INC_FILE { oui_c_include($1); }
	;

preceed : KW_PRECEED pre_package
	| preceed pre_package
	;
pre_package : package { oui_prepkg($1); }
	;

follow : KW_FOLLOW fol_package
	| follow fol_package
	;
fol_package : package { oui_folpkg($1); }
	;
	
package : TK_PACKAGE { $$ = find_package($1); free_memory($1); }
	;
