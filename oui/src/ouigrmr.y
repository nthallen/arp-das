%{
  /* $Log$
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
%token KW_C_INCLUDE
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
statement : KW_OPTS TK_STRING { oui_opts($2); }
	| KW_C_INCLUDE TK_INC_FILE { oui_c_include($2); }
	| KW_DEF TK_STRING { oui_defs($2); }
	| KW_VAR TK_STRING { oui_vars($2); }
	| KW_INIT TK_STRING { oui_inits($2); }
	| KW_UNSORT TK_STRING { oui_unsort($2); }
	| KW_SYNOPSIS TK_STRING { oui_synopsis($2); }
	| KW_SORT TK_STRING { oui_sort($2); }
	| KW_SWITCH TK_STRING { oui_switch($2); }
	| KW_ARG TK_STRING { oui_arg($2); }
	| include
	| preceed
	| follow
	;
include : KW_INCLUDE inc_file
	| include inc_file
	;
inc_file : TK_PACKAGE { oui_include($1); }
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
