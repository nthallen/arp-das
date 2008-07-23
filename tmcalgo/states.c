/* states.c */
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "nortlib.h"
#include "compiler.h"
#include "yytype.h"
#include "dot.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

/* Check the new name against the list of names already defined */
static struct nmlst {
  struct nmlst *next;
  char *name;
} *namelist = NULL;

char *new_state_name(char *name) {
  struct nmlst *nl;

  for (nl = namelist; nl != NULL; nl = nl->next) {
	if (stricmp(nl->name, name) == 0) {
	  compile_error(2, "Duplicate State Name");
	  break;
	}
  }
  if (nl == NULL) {
	nl = new_memory(sizeof(struct nmlst));
	nl->next = namelist;
	nl->name = name;
	namelist = nl;
  }
  return(name);
}

#define LINE_LENGTH_LIMIT 70
static int state_listed = 0, line_length = 0, word_length;

/* list_state adds one state name to the current state
   declaration, taking into account line lengths, etc.
*/
static void list_state(FILE *ofp, const char *name) {
  if (!state_listed) {
	fprintf(ofp, "state (");
	line_length = LINE_LENGTH_LIMIT - 7;
  } else {
	fputc(',', ofp);
	line_length--;
  }
  word_length = strlen(name);
  if (state_listed) word_length++;
  line_length -= word_length;
  if (line_length < 0) {
	fprintf(ofp, "\n");
	line_length = LINE_LENGTH_LIMIT - word_length;
  }
  if (state_listed) fputc(' ', ofp);
  else state_listed = 1;
  fprintf(ofp, "%s", name);
}

/* end_state() ends the current state declaration (if one
   has in fact been started).
*/
static void end_state(FILE *ofp, char *first_state) {
  if (state_listed) {
	fprintf(ofp, ");\n");
	if (first_state != NULL)
	  fprintf(ofp, "Validate %s;\n", first_state);
	state_listed = 0;
  }
}

typedef struct state_case_st {
  struct state_case_st *next;
  char *statename;
  int statecase;
  int partno;
  int slurp;
} state_case;
static struct {
  state_case *first;
  state_case *last;
} state_cases;
static int gonna_slurp;

static state_case *find_state_case( char *statename ) {
  state_case *sc;
  static int last_case = 0;

  for ( sc = state_cases.first; sc != 0; sc = sc->next ) {
	if ( strcmp( statename, sc->statename ) == 0 )
	  break;
  }
  if ( sc == NULL ) {
	sc = new_memory( sizeof( state_case ) );
	sc->next = NULL;
	sc->statename = statename;
	sc->statecase = ++last_case;
	sc->slurp = 0;
	sc->partno = -1;
	if ( state_cases.last == 0 ) state_cases.first = sc;
	else state_cases.last->next = sc;
	state_cases.last = sc;
  }
  return sc;
}

int get_state_case( char *statename, int slurp, int partno ) {
  state_case *sc;
  
  sc = find_state_case( statename );
  if ( slurp ) sc->slurp = 1;
  if ( partno >= 0 ) sc->partno = partno;
  return sc->statecase;
}

static int get_state_partno( char *statename ) {
  state_case *sc;

  sc = find_state_case( statename );
  return sc->partno;
}

/* new substate lists the new substate (list_state()) and creates
   a structure to hold the substate's name, whether it is an 
   internal substate (once) and the case value which will refer 
   to this substate.
*/
static struct prtn *curr_partition;

/* dup_substate returns struct substate that is the same
   as the old one, but might have a different 'once'
   qualifier. I'm optimizing here by returning the same
   substate if once hasn't changed.
*/
struct substate *dup_substate( struct substate *ss, int once ) {
  struct substate *newsub;

  if ( once == ss->once ) return ss;
  newsub = new_memory( sizeof( struct substate ) );
  /* newsub->next = NULL; */
  newsub->name = ss->name;
  newsub->once = once;
  newsub->state_case = ss->state_case;
  return newsub;
}

struct substate *new_substate( FILE *ofp, char *name,
		  int once, int mkcase ) {
  char *substate;
  struct substate *newsub;

  substate = strdup( name );
  assert( curr_partition != 0 );
  if ( ofp != 0 ) list_state( ofp, substate );
  newsub = new_memory( sizeof( struct substate ) );
  /* newsub->next = NULL; */
  newsub->name = substate;
  newsub->once = once;
  newsub->state_case =
	mkcase ? get_state_case( substate, 0, -1 ) : -1;
  return newsub;
}

static int imp_index;
static char * imp_name;
static struct substate *new_imp_substate( FILE *ofp,
		int do_once, int end ) {
  char buf[80];
  
  assert( imp_name != 0 );
  if ( end )
	sprintf( buf, "%s_end_", imp_name );
  else
	sprintf( buf, "%s_%d_", imp_name, imp_index++ );
  return new_substate( ofp, buf, do_once, 1 );
}
static void init_imp_substate( char *name ) {
  imp_index = 1;
  imp_name = name;
}

static int needs_substate( struct cmddef *cmd ) {
  if ( cmd == 0 ) return 0;
  switch ( cmd->cmdtype ) {
	case CMDTYPE_TMC:
	case CMDTYPE_HOLD:
	case CMDTYPE_VHOLD:
	  return 1;
	case CMDTYPE_QSTR:
	case CMDTYPE_CMD:
	case CMDTYPE_VAL:
	case CMDTYPE_HOLDV:
	case CMDTYPE_RES:
	  return 0;
	default:
	  compile_error( 4, "Unknown command type %d", cmd->cmdtype );
	  return 0;
  }
}

static int is_hold( struct cmddef *cmd ) {
  if ( cmd == 0 ) return 0;
  switch ( cmd->cmdtype ) {
	case CMDTYPE_HOLD:
	case CMDTYPE_VHOLD:
	case CMDTYPE_HOLDV:
	  return 1;
	default:
	  return 0;
  }
}

static int count_else( FILE *ofp, struct cmddef *cmd,
				  struct substate **prev_substate ) {
  int retcnt = 2;
  cmd = cmd->else_stat;
  if ( cmd == 0 ) return 1;
  if ( needs_substate( cmd ) ) {
	*prev_substate = cmd->substate =
	  new_imp_substate( ofp, cmd->cmdtype == CMDTYPE_TMC, 0 );
	if ( cmd->cmdtype == CMDTYPE_TMC )
	  cmd->cmdtype = CMDTYPE_SS;
  } else if ( cmd->cmdtype == CMDTYPE_HOLDV ) {
	if ( (*prev_substate)->state_case !=
		 curr_partition->idle_substate->state_case ) {
	  cmd->substate = curr_partition->idle_substate;
	  if ( cmd->cmdtext ) retcnt++;
	}
  }
  if ( is_hold( cmd ) ) {
	cmd->else_count = count_else( ofp, cmd, prev_substate );
	retcnt += cmd->else_count - 1;
  }
  return retcnt;
}

/* list_substates() outputs sub-state declarations for
   one state. Each partition will have at least one idle 
   substate, but we don't require any default substates per 
   state. We create a substate only if there are TMC commands for 
   the specified time period (including _HOLD and _VHOLD).

   * The following is not true. Always statically validate the
     idle substate. at T=0, the appropriate substate will be
	 validated ***********************************************
   A substate should be statically validated if it is at T=0 in 
   the first state in the partition. If there is no such 
   substate, the partition's idle substate should be validated.
*/
static void list_substates( FILE *ofp, struct stdef *state ) {
  struct cmddef *cmd, *ncmd, *icmd, *prev_ss_idle = NULL;
  struct substate *prev_substate;

  init_imp_substate( state->name );
  /* Skip over top commands */
  for (cmd = state->cmds; cmd != NULL && cmd->cmdtime < 0;
		cmd = cmd->next) {
	if ( cmd->cmdtype == CMDTYPE_TMC )
	  cmd->substate = new_substate( NULL, state->name, 0, 0 );
	else compile_error( 4,
		"Unexpected CMDTYPE %d at time < 0", cmd->cmdtype );
  }
  prev_substate = curr_partition->idle_substate;
  
  #if 0
  Need to redo this search. The issue is where to put in
  CMDTYPE_SS elements. This pertains only to _TMC and _*HOLD*
  commands. When we encounter a _TMC, we want to place the
  relevant _SS at the end of the current "group", so we look
  ahead for a command at a later time or a _*HOLD*. We get
  3 cases: 1: no later commands (_end_ condition) 2: later
  commands 3: _*HOLD*. The last breaks down into _HOLDV
  and the other two cases and _HOLDV has two cases itself,
  depending on whether the V part is present. _HOLD and _VHOLD
  can use the same substate as the earlier _TMC. _HOLDV without
  the V can adopt the earlier substate, but otherwise requires
  insertion of the _SS. In all the _*HOLD* cases, we need to
  search through the else clauses to assign substates as
  necessary. We also need at insert an _SS to _idle_ after any
  _*HOLD*. This could be optimized out if the following group
  immediately defines a new substate, but I'll wait to see how
  that might work out. I think I won't search first for
  needs_substate(), since that leads me into trouble with
  _HOLDV, which doesn't. Instead, I'll identify every command
  group, searching for the last statement, then take a second
  pass assigning the substates, then look at the _*HOLD*s
  #endif

  while ( cmd != 0 ) {
	struct substate *tmc_ss = NULL;
	int is_internal;
	
	/* 1: identify the current command group, which
	      is all commands at the same time as the first
		  up to but not beyond any _*HOLD* statements.
	*/
	for ( ncmd = cmd;
		  ! is_hold( ncmd ) &&
		  ncmd->next != 0 &&
		  ncmd->next->cmdtime == cmd->cmdtime;
		  ncmd = ncmd->next );

	/* 2: Determine whether this subgroup is internal or
	      Terminal */
	is_internal = is_hold( ncmd ) || ncmd->next != 0;

	/* 3: Look for any _TMC statements */
	for ( icmd = cmd; ; icmd = icmd->next ) {
	  if ( needs_substate( icmd ) ) {
		icmd->substate = tmc_ss =
		  new_imp_substate( ofp,
			is_internal && ! is_hold( icmd ),
			! is_internal );
		while ( icmd != ncmd && icmd->next ) {
		  icmd = icmd->next;
		  if ( needs_substate( icmd ) ) {
			icmd->substate =
			  dup_substate( tmc_ss,
				is_internal && ! is_hold( icmd ) );
		  }
		};
		break;
	  }
	  if ( icmd == ncmd ) break;
	}
	
	/* 4: If no substate identified, set substate to
		  _idle */
	if ( tmc_ss == 0 ) tmc_ss = curr_partition->idle_substate;
	else if ( prev_ss_idle &&
			  prev_ss_idle->cmdtime == ncmd->cmdtime ) {
	  struct cmddef *old = prev_ss_idle->next;
	  prev_ss_idle->next = old->next;
	  free_memory( old );
	}
	prev_ss_idle = NULL;
	
	/* 4: Process _*HOLD* statements */
	if ( is_hold( ncmd ) ) {
	  if ( ncmd->cmdtype == CMDTYPE_HOLDV ) {
		if ( tmc_ss->state_case != prev_substate->state_case )
		  ncmd->substate = tmc_ss;
	  }
	  prev_substate = tmc_ss;
	  tmc_ss = curr_partition->idle_substate;
	  ncmd->else_count = count_else( ofp, ncmd, &prev_substate );
	}
	if ( tmc_ss->state_case != prev_substate->state_case ) {
	  struct cmddef *oldcmd = ncmd;
	  ncmd = new_command( CMDTYPE_SS, NULL, NULL );
	  ncmd->cmdtime = oldcmd->cmdtime;
	  ncmd->substate = tmc_ss;
	  ncmd->next = oldcmd->next;
	  oldcmd->next = ncmd;
	  prev_substate = tmc_ss;
	  if ( tmc_ss->state_case ==
		   curr_partition->idle_substate->state_case )
		prev_ss_idle = oldcmd;
	}
	cmd = ncmd->next;
  }
}

static void end_substates( FILE *ofp ) {
  if ( curr_partition->partno >= 0 ) {
	assert( curr_partition->idle_substate != 0 );
	end_state( ofp, curr_partition->idle_substate->name );
  }
}

/* List States outputs top-level state () declarations
   and sub-state declarations for each partition.
*/
void list_states(FILE *ofp) {
  struct prg *pi;
  char *first_state = NULL;
  int partno = 0;

  for (pi = program; pi != NULL; pi = pi->next) {
	switch (pi->type) {
	  case PRGTYPE_STATE:
		assert( curr_partition != NULL );
		if ( ! state_listed && dot_fp ) {
		  fprintf( dot_fp, "  subgraph cluster_%d {\n"
			"    label=\"Partition %d\";\n", partno, partno+1 );
		}
		if ( dot_fp ) {
		  int isslurp = pi->u.state->filename != 0;

		  fprintf( dot_fp, "    %s", pi->u.state->name );
		  if ( first_state == 0 ) {
			fprintf( dot_fp, " [style=filled" );
			if ( isslurp ) fprintf( dot_fp, ",peripheries=2" );
			fprintf( dot_fp, "]" );
		  } else if ( isslurp ) {
			fprintf( dot_fp, " [peripheries=2]" );
		  }
		  fprintf( dot_fp, ";\n" );
		}
		list_state(ofp, pi->u.state->name);
		if ( first_state == 0 )
		  first_state = pi->u.state->name;
		/* Make states available to slurp files: */
		get_state_case( pi->u.state->name, 1, partno );
		break;
	  case PRGTYPE_PARTITION:
		if ( curr_partition != 0 && state_listed )
		  curr_partition->partno = partno++;
		if ( dot_fp && state_listed )
		  fprintf( dot_fp, "  }\n" );
		end_state(ofp, first_state);
		first_state = NULL;
		curr_partition = pi->u.partition;
		break;
	}
  }
  if ( curr_partition != 0 && state_listed )
	curr_partition->partno = partno++;
  if ( dot_fp && state_listed )
	fprintf( dot_fp, "  }\n" );
  end_state(ofp, first_state);
  
  /* now list the substates for each partition */
  curr_partition = NULL;
  for (pi = program; pi != NULL; pi = pi->next) {
	switch (pi->type) {
	  case PRGTYPE_STATE:
		list_substates(ofp, pi->u.state );
		break;
	  case PRGTYPE_PARTITION:
		if ( curr_partition != 0 )
		  end_substates( ofp );
		curr_partition = pi->u.partition;
		assert( curr_partition->idle_substate == 0 );
		{ char buf[40];
		  sprintf( buf, "part_%d_idle_", curr_partition->partno );
		  curr_partition->idle_substate =
			new_substate( ofp, buf, 0, 1 );
		}
		break;
	}
  }
  end_substates( ofp );
}

static void output_dependency( FILE *ofp,
	struct substate *substate, char *depends ) {
  fprintf( ofp, "depending on ( " );
  if ( substate != 0 ) {
	fprintf( ofp, "%s", substate->name );
	if ( substate->once )
	  fprintf( ofp, " once" );
	if ( depends != 0 ) fprintf( ofp, ", " );
	else putc( ' ', ofp );
  }
  if ( depends != 0 )
	fprintf( ofp, "%s ", depends );
  fprintf( ofp, ") " );
}

static void output_tmccmd( FILE *ofp, struct cmddef *cmd ) {
  int is_val, has_deps;
  
  assert( cmd != 0 );
  assert( cmd->cmd2text != 0 || cmd->cmdtext != 0 );
  assert( cmd->cmdtype == CMDTYPE_TMC || cmd->cmdtype == CMDTYPE_SS );
  is_val = ( cmd->cmd2text == 0 ); /* If this is a validation */
  assert( ! is_val ); /* obsolete usage? Now CMDTYPE_VAL */
  has_deps = ( ! is_val ) && cmd->cmdtext != 0;

  /* I'm guessing the following is true: */
  assert( cmd->substate == 0 || ! is_val );

  if ( cmd->substate != 0 || has_deps ) {
	output_dependency( ofp, cmd->substate, cmd->cmdtext );
	if ( is_val ) fprintf( ofp, "Validate %s;\n", cmd->cmdtext );
	else fprintf( ofp, "{%s}\n", cmd->cmd2text );
  } else if ( is_val )
	fprintf( ofp, "Validate %s;\n", cmd->cmdtext );
  else fprintf( ofp, "%s\n", cmd->cmd2text );
}

static void output_hold( FILE *ofp, struct cmddef *cmd ) {
  output_dependency( ofp, cmd->substate, NULL );
  fprintf( ofp, "{\n  if (%s) {\n", cmd->cmdtext );
  fprintf( ofp,
	"    tma_succeed( %d, %d );\n"
	"    validate %s;\n"
	"  }\n"
	"}\n",
	curr_partition->partno, cmd->substate->state_case,
	curr_partition->idle_substate->name );
}

static void output_vhold( FILE *ofp, struct cmddef *cmd ) {
  output_dependency( ofp, cmd->substate, cmd->cmdtext );
  fprintf( ofp,
	"{\n"
	"  tma_succeed( %d, %d );\n"
	"  validate %s;\n"
	"}\n",
	curr_partition->partno, cmd->substate->state_case,
	curr_partition->idle_substate->name );
}

#define DO_BACK_LINK
static void output_edge( char *from, char *to, char *attr ) {
  int partno;
  
  if ( dot_fp == NULL ) return;
  if ( attr == NULL ) {
	partno = get_state_partno( to );
	if ( partno == curr_partition->partno ) {
	  #ifdef DO_BACK_LINK
	  int fsc, tsc;
	  fsc = get_state_case( from, 0, -1 );
	  tsc = get_state_case( to, 0, -1 );
	  if ( fsc > tsc ) {
		char *hold;
		hold = from;
		from = to;
		to = hold;
		attr = " [dir=back]";
	  } else attr = "";
	  #else
	  attr = "";
	  #endif
	} else {
	  attr = " [color=red]";
	}
  }
  fprintf( dot_fp, "  %s -> %s%s;\n", from, to, attr );
}

static void check_for_validate( char *from, char *tmccmd ) {
  int n;
  char *s = tmccmd;
  if ( dot_fp == NULL ) return;
  while ( *s ) {
	while ( *s && tolower(*s) != 'v' ) s++;
	if ( *s == '\0' ) return;
	if ( strnicmp( s, "validate", 8 ) == 0 ) {
	  char to[80];
	  s += 8;
	  if ( ! isspace(*s) ) continue;
	  while ( isspace(*s) ) s++;
	  if ( *s != '_' && ! isalpha(*s) ) continue;
	  for ( n = 0; n < 80 && ( isalnum(s[n]) || s[n] == '_' ); n++ ) {
		to[n] = s[n];
	  }
	  if ( n >= 80 ) {
		nl_error( 2,
		  "Identifier exceeds internal buffer in dot processing" );
		return;
	  }
	  to[n] = '\0';
	  output_edge( from, to, NULL );
	  s += n;
	} else s++;
  }
}

static void output_cmd_code( FILE *ofp, struct cmddef *cmd,
  struct stdef *state ) {
  assert( cmd != 0 );
  switch ( cmd->cmdtype ) {
	case CMDTYPE_CMD:
	  fprintf( ofp, "%8ld, \">%s\\n\",\n", cmd->cmdtime, cmd->cmdtext );
	  break;
	case CMDTYPE_VAL:
	  fprintf( ofp, "%8ld, \"#%d\", /* %s */\n", cmd->cmdtime,
		get_state_case( cmd->cmdtext, 0, -1 ), cmd->cmdtext );
	  output_edge( state->name, cmd->cmdtext, NULL );
	  break;
	case CMDTYPE_QSTR:
	  fprintf( ofp, "%8ld, \"\\%s,\n", cmd->cmdtime, cmd->cmdtext );
	  break;
	case CMDTYPE_HOLD:
	case CMDTYPE_VHOLD:
	case CMDTYPE_HOLDV:
	  { int mycase = get_state_case( state->name, 0, -1 );
		
		assert( cmd->cmdtype == CMDTYPE_HOLDV || cmd->substate != 0 );
		if ( cmd->cmdtype == CMDTYPE_HOLDV ) {
		  int statecase = 0;
		  char *statetext = "Hold";
		  if ( cmd->cmdtext != NULL ) {
			statecase = get_state_case( cmd->cmdtext, 0, -1 );
			statetext = cmd->cmdtext;
			output_edge( state->name, cmd->cmdtext, NULL );
			if ( cmd->substate ) {
			  fprintf( ofp, "%8ld, \"#%d\", /* %s */\n",
				cmd->cmdtime, cmd->substate->state_case,
				cmd->substate->name );
			}
		  } else if ( cmd->substate ) {
			statecase = cmd->substate->state_case;
			statetext = cmd->substate->name;
		  }
		  fprintf( ofp, "%8ld, \"?%d,%ld,%d,%d\", /* %s */\n",
			cmd->cmdtime, cmd->else_count, cmd->timeout,
			statecase, mycase, statetext );
		} else {
		  fprintf( ofp, "%8ld, \"?%d,%ld,%d,%d\", /* %s */\n", 
			cmd->cmdtime, cmd->else_count, cmd->timeout,
			cmd->substate->state_case, mycase, cmd->substate->name );
		}
		if ( cmd->else_stat )
		  output_cmd_code( ofp, cmd->else_stat, state );
	  }
	  break;
	case CMDTYPE_RES:
	  fprintf( ofp, "%8ld, \"R%d,%d\", /* %s */\n",
		cmd->cmdtime,
		get_state_partno( cmd->cmdtext ),
		get_state_case( cmd->cmdtext, 0, -1 ),
		cmd->cmdtext );
	  output_edge( state->name, cmd->cmdtext, " [color=blue]" );
	  break;
	case CMDTYPE_SS:
	  fprintf( ofp, "%8ld, \"#%d\", /* %s */\n",
		cmd->cmdtime,
		cmd->substate->state_case,
		cmd->substate->name );
	  break;
	case CMDTYPE_TMC:
	  assert( cmd->substate != 0 );
	  check_for_validate( state->name, cmd->cmd2text );
	  /* I suppress this output here and generate it
	     in output_state() instead */
	  /* fprintf( ofp, "%8ld, \"#%d\", /\* %s *\/\n", cmd->cmdtime,
		cmd->substate->state_case, cmd->substate->name ); */
	  break;
	default:
	  compile_error( 4,
		"Unexpected cmdtype %d in output_cmd_code",
		cmd->cmdtype );
  }
}

/* Output one top-level state */
static void output_state(FILE *ofp, struct stdef *state ) {
  struct cmddef *cmd, *cmd0;

  /* Skip the T=-1 conditions */
  for (cmd = state->cmds; cmd != 0 && cmd->cmdtime < 0;
					cmd = cmd->next) {
	assert(cmd->cmdtype == CMDTYPE_TMC);
	check_for_validate( state->name, cmd->cmd2text );
  }

  /* Define the state's command structure */
  fprintf( ofp, "%%{\n" );
  
  /* first print the commands */
  fprintf( ofp, "  tma_state %s_cmds_[] = {\n", state->name );
  assert( curr_partition != 0 );
  assert( curr_partition->idle_substate != 0 );
  for ( ; cmd; cmd = cmd->next ) {
	output_cmd_code( ofp, cmd, state );
  }
  fprintf( ofp, "%8ld, NULL\n  };\n", -1L );
  
  if (state->filename != 0 ) {
	fprintf( ofp, "  tma_ifile %s_file_ = {\n", state->name );
	fprintf( ofp, "    %d, %s, \"%s%s\", NULL, %s_cmds_, -1\n  };\n",
	  curr_partition->partno, state->filename,
	  state->nolog ? "_" : "", state->name, 
	  state->name );
	fprintf( ofp, "  tma_ifile *%s_filep_ = & %s_file_;\n",
	  state->name, state->name );
  }
  fprintf( ofp, "%%}\n" );

  /* output the initialization */
  fprintf( ofp, "depending on (%s once) {\n", state->name );
  if ( state->filename != 0 ) {
	fprintf( ofp, "  tma_read_file( %s_filep_ );\n", state->name );
	gonna_slurp = 1;
  } else {
	fprintf( ofp, "  tma_init_state( %d, %s_cmds_, \"%s%s\" );\n",
			  curr_partition->partno, state->name,
			  state->nolog ? "_" : "",  state->name );
  }
  fprintf( ofp, "  Validate %s;\n}\n", 
			curr_partition->idle_substate->name );

  /* output the substate commands */
  for ( cmd = state->cmds; cmd != 0; cmd = cmd->next ) {
	for ( cmd0 = cmd; cmd0 != 0; cmd0 = cmd0->else_stat ) {
	  switch ( cmd0->cmdtype ) {
		case CMDTYPE_SS: if ( ! cmd0->cmd2text ) break;
		case CMDTYPE_TMC: output_tmccmd( ofp, cmd0 ); break;
		case CMDTYPE_HOLD: output_hold( ofp, cmd0 ); break;
		case CMDTYPE_VHOLD: output_vhold( ofp, cmd0 ); break;
	  }
	}
  }
}

/* Output the actual commands */
void output_states(FILE *ofp) {
  struct prg *pi;
  int max_partno = -1;

  for (pi = program; pi != NULL; pi = pi->next) {
	switch (pi->type) {
	  case PRGTYPE_STATE:
		output_state(ofp, pi->u.state);
		break;
	  case PRGTYPE_TMCCMD:
		output_tmccmd( ofp, pi->u.tmccmd );
		break;
	  case PRGTYPE_PARTITION:
		curr_partition = pi->u.partition;
		if ( curr_partition->partno > max_partno )
		  max_partno = curr_partition->partno;
		break;
	}
  }
  fprintf(ofp, "%%{\n"
	"  const int tma_n_partitions = %d;\n"
	"%%}\n", max_partno + 1 );
  if ( dot_fp ) fprintf( dot_fp, "}\n" );
}

void output_mainloop( FILE *ofp ) {
  state_case *subs;
  fprintf( ofp, "depending on ( 1 Hz ) {\n"
				"  long int it;\n"
				"  int subcase;\n\n"
				"  it = itime();\n"
				"  ci_settime( it );\n"
				"  while ( subcase = tma_process( it ) ) {\n"
				"\tswitch (subcase) {\n" );
  fprintf( ofp, "\t  case -1:\n"
				"\t  case 0: break;\n" );
  for ( subs = state_cases.first; subs != 0; subs = subs->next ) {
	fprintf( ofp, "\t  case %d: validate %s; break;\n",
	  subs->statecase, subs->statename );
  }
  fprintf( ofp, "\t  default:\n"
                "\t\tnl_error( 1, "
                "\"Unexpected return value from tma_process\" );\n"
				"\t\tbreak;\n"
				"\t}\n"
				"  }\n"
				"}\n" );
  if ( gonna_slurp ) {
	fprintf( ofp, "%%{\n  slurp_val slurp_vals[] = {\n" );
	for ( subs = state_cases.first; subs != 0; subs = subs->next ) {
	  if ( subs->slurp ) {
		fprintf( ofp, "    \"%s\", \"#%d",
		  subs->statename, subs->statecase );
		if ( subs->partno >= 0 ) {
		  fprintf( ofp, "R%d,%d",
			subs->partno, subs->statecase );
		}
		fprintf( ofp, "\",\n" );
	  }
	}
	fprintf( ofp, "    NULL, 0\n  };\n%%}\n" );
  }
}
