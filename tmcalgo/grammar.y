%{
  /* grammar.y Grammar for tmcalgo
   * $Log$
   */
  #include <stdio.h>
  #include <assert.h>
  #include "nortlib.h"
  #include "yytype.h"
  #include "y.tab.h"
  #include "memlib.h"
  static char rcsid[] = "$Id$";

  #define yyerror(x) nl_error(2, x)
  #define PARSE_DEBUG (-2)
  static long int last_command_time;
  
  static struct cmddef *new_command(int flags, char *text) {
	struct cmddef *cd;
	
	cd = new_memory(sizeof(struct cmddef));
	cd->next = NULL;
	cd->cmdtime = 0L;
	cd->cmdtext = text;
	cd->cmdflags = flags;
	return(cd);
  }
  
  static void append_command(struct cmdlst *cl, struct cmddef *cmd) {
	if (cl->first == NULL) cl->first = cl->last = cmd;
	else {
	  assert(cl->last != NULL);
	  if (cl->last->cmdtime == cmd->cmdtime &&
		  (cl->last->cmdflags & CMDFLAGS_TMC) &&
		  (cmd->cmdflags & CMDFLAGS_TMC) == 0)
		nl_error(2, "TMC Command must follow all commands for time");
	  cl->last = cl->last->next = cmd;
	}
  }
  
  static void append_prog(struct prglst *pl, struct prg *prog) {
	struct prg *pg;

	pg = new_memory(sizeof(struct prg));
	*pg = *prog;
	pg->next = NULL;
	if (pl->last == NULL) pl->first = pl->last = pg;
	else pl->last = pl->last->next = pg;
  }
  
  static struct stdef *new_state(char *name, struct cmddef *cmds) {
	struct stdef *ns;
	
	ns = new_memory(sizeof(struct stdef));
	ns->name = name;
	ns->cmds = cmds;
	return(ns);
  }
  
  struct prg *program;
%}

%token KW_STATE
%token KW_PARTITION
%token <textval>  TK_TMCSTAT
%token <textval>  TK_NAME
%token <textval>  TK_COMMAND
%token <intval>   TK_INTEGER
%type  <intval>   time
%type  <intval>   time_spec
%type  <cmdval>   command
%type  <cmdval>   tmc_statement
%type  <cmdsval>  timed_commands
%type  <cmdsval>  untimed_commands
%type  <textval>  state_name_def
%type  <stateval> state_def 
%type  <prgval>   prog_item
%type  <pglval>   prog_items

%%
Program : prog_items { program = $1.first; }
	;
prog_items : { $$.first = $$.last = NULL; }
	| prog_items prog_item {
	  $$ = $1;
	  append_prog(&$$, &$2);
	}
	;
prog_item : state_def {
	  $$.type = PRGTYPE_STATE;
	  $$.u.state = $1;
	}
	| TK_TMCSTAT {
	  $$.type = PRGTYPE_TMCCMD;
	  $$.u.tmccmd = $1;
	}
	| KW_PARTITION { $$.type = PRGTYPE_PARTITION; }
	;
state_def : KW_STATE state_name_def '{' untimed_commands timed_commands '}' {
	  nl_error(PARSE_DEBUG, "State Definition %s", $2);
	  if ($4.first == NULL) $4.first = $5.first;
	  else $4.last->next = $5.first;
	  $$ = new_state($2, $4.first);
	}
	;
state_name_def : TK_NAME { $$ = new_state_name($1); }
	;
untimed_commands : {
	  last_command_time = -1;
	  $$.first = $$.last = NULL;
	}
	| untimed_commands tmc_statement {
	  $$ = $1;
	  $2->cmdtime = -1;
	  append_command(&$$, $2);
	}
timed_commands : {
	  last_command_time = 0;
	  $$.first = $$.last = NULL;
	}
	| timed_commands time_spec command {
	  $$ = $1;
	  $3->cmdtime = $2;
	  append_command(&$$, $3);
	}
	;
time_spec : { $$ = last_command_time; }
	| time {
	  if ($1 < last_command_time)
		nl_error(2, "Command Time preceeds previous command");
	  $$ = last_command_time = $1;
	}
	| '+' time { $$ = last_command_time += $2; }
	;
time : TK_INTEGER { $$ = $1; }
	| time ':' TK_INTEGER { $$ = $1 * 60 + $3; }
	;
command : TK_COMMAND {
	  nl_error(PARSE_DEBUG, "Read command \"%s\"", $1);
	  check_command($1);
	  $$ = new_command(0, $1);
	}
	| tmc_statement
	;
tmc_statement : TK_TMCSTAT {
	  nl_error(PARSE_DEBUG, "Saw statement:\n%s", $1);
	  $$ = new_command(CMDFLAGS_TMC, $1);
	}
	;
