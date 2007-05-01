%{
  /* $Log$
  /* Revision 1.4  2004/10/08 17:07:20  nort
  /* Mostly keyword differences
  /*
 * Revision 1.3  1995/05/25  17:18:37  nort
 * Use standard nortlib compiler functions
 *
 * Revision 1.2  1993/05/18  13:07:37  nort
 * Changes for client/server support
 *
 * Revision 1.1  1992/07/09  18:36:44  nort
 * Initial revision
 * */
  #include <stdio.h>
  #include <stdlib.h>
  #include <stdarg.h>
  #include <assert.h>
  #include <ctype.h>
  #include "cmdgen.h"
  #include "compiler.h"
  
  #pragma off (unreferenced)
	static char rcsid[] =
	  "$Id$";
  #pragma on (unreferenced)
  #define yyerror(x) compile_error(2, x)
%}
%token <nt_val>  TK_NON_TERMINAL
%token <str_val> TK_TYPE_SPEC
%token <str_val> TK_WORD
%token <str_val> TK_VAR_SPEC
%token <str_val> TK_C_CODE
%token <str_val> TK_PROMPT
%token <str_val> TK_INTERFACE
%type <nt_val> Rule
%type <nt_val> nt_spec
%type <type_val> type_spec
%type <sub_val> sub_items
%type <subi_val> sub_item
%type <str_val> var_prmpt
%%
Rules     :
		  | Rules Rule ';' {
			  if ($2->rules.first == NULL)
				compile_error(3, "Non-terminal has no rules");
			}
		  | Rules TK_INTERFACE TK_TYPE_SPEC {
			  new_interface( $3 );
			}
		  ;
Rule	  : nt_spec
		  | Rule ':' sub_items {
			  /* If Rule is a Client nt, check for dummy nts
				 and make them client nts also.
			  */
			  if ($1->name[0] == '&') {
				struct sub_item_t *si;
				
				for (si = $3->items.first; si != NULL; si = si->next) {
				  if (si->type == SI_NT && si->u.nt->name[0] == '_')
					si->u.nt->name[0] = '&';
				}
			  }
			  if ($1->rules.last == NULL)
				$1->rules.first = $1->rules.last = $3;
			  else $1->rules.last = $1->rules.last->next = $3;
			  $3->reduces = $1;
			  $$ = $1;
			}
		  ;
nt_spec	  : TK_NON_TERMINAL type_spec {
			  if ($2 != NULL) {
			    if ($1->type != NULL)
				  compile_error(2, "Attempted redefinition of type for %s",
						  $1->name);
				$1->type = $2;
			  }
			  $$ = $1;
			}
		  ;
type_spec : { $$ = NULL; }
		  | TK_TYPE_SPEC { $$ = get_vtype($1); }
		  ;
sub_items : { $$ = new_sub(); }
          | sub_items sub_item {
			  if ($1->action != NULL) dmy_non_term($1);
			  if ($1->items.last == NULL)
				$1->items.first = $1->items.last = $2;
			  else $1->items.last = $1->items.last->next = $2;
			  $$ = $1;
			}
		  | sub_items TK_C_CODE {
			  if ($1->action != NULL) dmy_non_term($1);
			  $1->action = $2;
			  $$ = $1;
			}
		  ;
sub_item  : TK_WORD {
			  char *s;
			  
			  for (s = $1; *s != '\0'; s++)
				if (!isprint(*s))
				  compile_error(3, "Illegal character 0x%02X", *s);
			  $$ = new_sub_item(SI_WORD);
			  $$->u.text = $1;
			}
		  | '*' {
			  $$ = new_sub_item(SI_WORD);
			  $$->u.text = "\n";
			}
		  | TK_NON_TERMINAL {
			  $$ = new_sub_item(SI_NT);
			  $$->u.nt = $1;
			}
		  | TK_VAR_SPEC var_prmpt {
			  $$ = new_sub_item(SI_VSPC);
			  get_vsymbol($$, $1, $2);
			}
		  ;
var_prmpt : { $$ = NULL; }
		  | TK_PROMPT { $$ = $1; }
		  ;
