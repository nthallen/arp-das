%{
  /* grammar.y Grammar for tmcalgo
   * $Log$
   * Revision 1.7  1999/12/03 16:46:11  nort
   * Added NoLog option to state definition
   *
   * Revision 1.6  1997/01/16 16:39:50  nort
   * All the new syntax
   *
 * Revision 1.5  1996/04/19  13:51:02  nort
 * Added semicolon after QSTRING
 *
 * Revision 1.4  1996/04/17  02:51:27  nort
 * Changes to support default commands in file slurps.
 *
 * Revision 1.2  1993/09/28  16:15:13  nort
 * Cleanup, added some comments
 *
 * Revision 1.1  1993/05/18  20:37:18  nort
 * Initial revision
 *
   */
  #include <stdio.h>
  #include <assert.h>
  #include "nortlib.h"
  #include "yytype.h"
  #include "y.tab.h"
  /* #include "memlib.h" */
  #pragma off (unreferenced)
	static char rcsid[] =
	  "$Id$";
  #pragma on (unreferenced)

  #define yyerror(x) nl_error(2, x)
  #define PARSE_DEBUG (-2)
  static long int last_command_time;
  
  static struct cmddef *
  new_command(int type, char *text, char *text2 ) {
	struct cmddef *cd;
	
	cd = new_memory(sizeof(struct cmddef));
	cd->next = NULL;
	cd->cmdtime = 0L;
	cd->substate = NULL;
	cd->cmdtext = text;
	cd->cmd2text = text2;
	cd->cmdtype = type;
	cd->timeout = 0;
	cd->else_stat = NULL;
	return(cd);
  }
  
  static void append_command(struct cmdlst *cl, struct cmddef *cmd) {
	if (cl->first == NULL) cl->first = cl->last = cmd;
	else {
	  assert(cl->last != NULL);
	  /* eliminated sorting test here. Sort on output? */
	  /* nl_error(2, "TMC Command must follow all commands for time"); */
	  cl->last = cl->last->next = cmd;
	}
  }
  
  /* append_prog() duplicates the specified struct prg and 
	 appends the copy to the specified struct prglst
  */
  static void append_prog(struct prglst *pl, struct prg *prog) {
	struct prg *pg;

	pg = new_memory(sizeof(struct prg));
	*pg = *prog;
	pg->next = NULL;
	if (pl->last == NULL) pl->first = pl->last = pg;
	else pl->last = pl->last->next = pg;
  }
  
  static struct stdef *new_state(char *name, struct cmddef *cmds,
			char *fname, int nolog ) {
	struct stdef *ns;
	
	ns = new_memory(sizeof(struct stdef));
	ns->name = name;
	ns->cmds = cmds;
	ns->filename = fname;
	ns->nolog = nolog;
	return(ns);
  }
  
  static struct prtn *new_partition( void ) {
	struct prtn *part;
	
	part = new_memory( sizeof( struct prtn ) );
	part->partno = -1;
	part->idle_substate = NULL;
	return part;
  }

  struct prg *program;
%}

%token KW_STATE
%token KW_PARTITION
%token KW_DEPENDING
%token KW_ON
%token KW_VALIDATE
%token KW_VALID
%token KW_HOLD
%token KW_UNTIL
%token KW_OR
%token KW_ELSE
%token KW_NOLOG
%token <textval>  TK_TMCSTAT
%token <textval>  TK_NAME
%token <textval>  TK_COMMAND
%token <textval>  TK_PARENSTAT
%token <textval>  TK_QSTRING
%token <intval>   TK_INTEGER
%type  <intval>   time
%type  <intval>   time_spec
%type  <cmdval>   timed_command
%type  <cmdsval>  timed_commands
%type  <cmdval>   untimed_command
%type  <cmdsval>  untimed_commands
%type  <cmdsval>  state_cmds
%type  <cmdval>   hold_clause
%type  <cmdval>   hold_cond
%type  <cmdval>   else_clause
%type  <intval>   opt_valid
%type  <intval>   opt_nolog
%type  <textval>  opt_state_file
%type  <textval>  state_name_def
%type  <stateval> state_def
%type  <prgval>   prog_item
%type  <prgval>   partn_def
%type  <pglval>   prog_items

%%
Program : prog_items { program = $1.first; }
	;
/* <pglval == struct prglst == first/last list of struct prg> */
prog_items : partn_def {
		$$.first = $$.last = NULL;
		append_prog( &$$, &$1 );
	  }
	| prog_items prog_item {
		$$ = $1;
		append_prog(&$$, &$2);
	  }
	;
/* <prgval == struct prg> */
prog_item : state_def {
		$$.type = PRGTYPE_STATE;
		$$.u.state = $1;
	  }
	| KW_PARTITION partn_def { $$ = $2; }
	| untimed_command {
		$$.type = PRGTYPE_TMCCMD;
		$$.u.tmccmd = $1;
	  }
	;
/* <prgval == struct prg> */
partn_def : {
		$$.type = PRGTYPE_PARTITION;
		$$.u.partition = new_partition();
	  }
	;
/* <stateval == struct stdef *> */
state_def : KW_STATE state_name_def opt_state_file opt_nolog state_cmds {
		nl_error(PARSE_DEBUG, "State Definition %s", $2);
		$$ = new_state( $2, $5.first, $3, $4 );
	  }
	;
/* opt_state_file == textval */
opt_state_file : { $$ = NULL; }
	| TK_QSTRING opt_state_list {
		$$ = $1;
		nl_error(PARSE_DEBUG, "State file %s", $1 );
	  }
	;
/* opt_nolog == intval */
opt_nolog : { $$ = 0; }
	| KW_NOLOG {
		nl_error(PARSE_DEBUG, "NoLog" );
		$$ = 1;
	  }
	;
/* state_cmds == cmdsval == struct cmdlst == first/last list of cmddef> */
state_cmds : '{' untimed_commands timed_commands '}' {
		nl_error(PARSE_DEBUG, "State_cmds" );
		if ($2.first == NULL) $2.first = $3.first;
		else $2.last->next = $3.first;
		$$ = $2;
	  }
	;
opt_state_list :
	| '(' state_list ')'
	;
state_list : state_list_elem
	| state_list ',' state_list_elem
	;
state_list_elem : TK_NAME { get_state_case( $1, 1, -1 ); }
	;
/* <textval == char *> */
state_name_def : TK_NAME {
		  $$ = new_state_name($1);
		  nl_error( PARSE_DEBUG, "state_name_def: %s", $1 );
		}
	;
/* <cmdsval == struct cmdlst == first/last list of cmddef> */
untimed_commands : {
		last_command_time = -1;
		$$.first = $$.last = NULL;
	  }
	| untimed_commands untimed_command {
		$$ = $1;
		$2->cmdtime = -1;
		append_command(&$$, $2);
	  }
	;
/* <cmdval == struct cmddef *> */
untimed_command : TK_TMCSTAT {
		nl_error(PARSE_DEBUG, "TMC: {%s}", $1 );
		$$ = new_command( CMDTYPE_TMC, NULL, $1 );
	  }
	| KW_DEPENDING KW_ON TK_PARENSTAT TK_TMCSTAT {
		nl_error(PARSE_DEBUG, "DEP: (%s) {%s}", $3, $4 );
		$$ = new_command( CMDTYPE_TMC, $3, $4 );
	  }
	;
/* <cmdsval == struct cmdlst == first/last list of cmddef> */
timed_commands : {
		last_command_time = 0;
		$$.first = $$.last = NULL;
	  }
	| timed_commands time_spec timed_command {
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

/* <cmdval == struct cmddef *> */
timed_command : TK_COMMAND {
		nl_error(PARSE_DEBUG, "Read command \"%s\"", $1);
		check_command($1);
		$$ = new_command( CMDTYPE_CMD, $1, NULL );
	  }
	| TK_QSTRING ';' {
		nl_error(PARSE_DEBUG, "Qstring %s", $1 );
		$$ = new_command( CMDTYPE_QSTR, $1, NULL );
	  }
	| KW_VALIDATE TK_NAME ';' {
		$$ = new_command( CMDTYPE_VAL, $2, NULL );
	  }
	| KW_HOLD KW_UNTIL hold_clause { $$ = $3; }
	| untimed_command { $$ = $1; }
	;
/* <cmdval == struct cmddef *> */
hold_clause : hold_cond ';' {
		$$ = $1;
		$$->timeout = -1;
		$$->else_stat = NULL;
	  }
	| hold_cond KW_OR time else_clause {
		$$ = $1;
		$$->timeout = $3;
		$$->else_stat = $4;
	  }
	;
/* <cmdval == struct cmddef *> */
hold_cond : opt_valid TK_PARENSTAT {
		$$ = new_command( $1, $2, NULL );
	  }
	;
/* <intval> */
opt_valid : { $$ = CMDTYPE_HOLD; }
	| KW_VALID { $$ = CMDTYPE_VHOLD; }
	;
/* <cmdval == struct cmddef *> */
else_clause : ';' { $$ = NULL; }
	| KW_ELSE timed_command { $$ = $2; }
	;
