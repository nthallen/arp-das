%{
  /* $Log$ */
  #include <stdio.h>
  #include <stdlib.h>
  #include <stdarg.h>
  #include <assert.h>
  #include <ctype.h>
  #include "cmdgen.h"
  
  #ifndef lint
	static char rcsid[] = "$Id$";
  #endif
  #define yyerror(x) app_error(2, x)
%}
%token <nt_val>  TK_NON_TERMINAL
%token <str_val> TK_TYPE_SPEC
%token <str_val> TK_WORD
%token <str_val> TK_VAR_SPEC
%token <str_val> TK_C_CODE
%token <str_val> TK_PROMPT
%token TK_END_OF_RULES
%type <nt_val> Rule
%type <type_val> type_spec
%type <sub_val> sub_items
%type <subi_val> sub_item
%type <str_val> var_prmpt
%%
Rules     :
		  | Rules Rule ';'
		  ;
Rule	  : TK_NON_TERMINAL type_spec ':' sub_items {
			  if ($2 != NULL) {
			    if ($1->type != NULL)
				  app_error(2, "Attempted redefinition of type for %s",
						  $1->name);
				$1->type = $2;
			  }
			  if ($1->rules.last == NULL)
				$1->rules.first = $1->rules.last = $4;
			  else $1->rules.last = $1->rules.last->next = $4;
			  $4->reduces = $1;
			  $$ = $1;
			}
		  | Rule ':' sub_items {
			  assert($1->rules.last != NULL);
			  $1->rules.last = $1->rules.last->next = $3;
			  $3->reduces = $1;
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
				  app_error(3, "Illegal character 0x%02X", *s);
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
%%

int error_level = 0, ignore_warnings = 0;
char *ifname = NULL;
int app_line = 1;

void app_error(unsigned int level, char *s, ...) {
  char *lvlmsg;
  va_list arg;

  fflush(stdout);  
  if (level > error_level) error_level = level;
  if (error_level == 1 && ignore_warnings) error_level = 0;
  va_start(arg, s);
  if (app_line > 0) {
	if (ifname != NULL) fprintf(efile, "%s %d:", ifname, app_line);
	else fprintf(efile, "%d:", app_line);
  }
  switch (level) {
    case 0: lvlmsg = "Info"; break;
	case 1: lvlmsg = "Warning"; break;
	case 2: lvlmsg = "Error"; break;
	case 3: lvlmsg = "Fatal"; break;
	default: lvlmsg = "Internal"; break;
  }
  fprintf(efile, "%s: ", lvlmsg);
  vfprintf(efile, s, arg);
  va_end(arg);
  fputc('\n', efile);
  if (level > 2) app_die(level);
}
