/* states.c
 * $Log$
 * Revision 1.10  1997/01/16 16:40:22  nort
 * New syntax
 *
 * Revision 1.9  1996/05/02  13:20:08  nort
 * *** empty log message ***
 *
 * Revision 1.8  1996/04/19  13:52:17  nort
 * Added all states to slurp list
 *
 * Revision 1.7  1996/04/17  02:51:54  nort
 * Rev 2 mods, including support for default commands in file slurps.
 *
 * Revision 1.6  1994/08/29  18:47:56  nort
 * Forgot tma_n_partitions definition
 * Revision 1.1  1993/05/18  20:37:21  nort
 * Initial revision
 */
#include <string.h>
#include <assert.h>
#include "nortlib.h"
#include "compiler.h"
#include "yytype.h"
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
  int slurp;
} state_case;
static struct {
  state_case *first;
  state_case *last;
} state_cases;
static int gonna_slurp;

int get_state_case( char *statename, int slurp ) {
  state_case *sc;
  static int last_case = 0;
  
  for ( sc = state_cases.first; sc != 0; sc = sc->next ) {
	if ( strcmp( statename, sc->statename ) == 0 )
	  break;
  }
  if ( sc == 0 ) {
	sc = new_memory( sizeof( state_case ) );
	sc->next = NULL;
	sc->statename = statename;
	sc->statecase = ++last_case;
	sc->slurp = 0;
	if ( state_cases.last == 0 ) state_cases.first = sc;
	else state_cases.last->next = sc;
	state_cases.last = sc;
  }
  if ( slurp ) sc->slurp = 1;
  return sc->statecase;
}

/* new substate lists the new substate (list_state()) and creates
   a structure to hold the substate's name, whether it is an 
   internal substate (once) and the case value which will refer 
   to this substate.
*/
static struct prtn *curr_partition;

struct substate *new_substate( FILE *ofp, char *name,
		  int once, int mkcase ) {
  char *substate;
  struct substate *newsub;

  substate = strdup( name );
  if ( ofp != 0 ) list_state( ofp, substate );
  newsub = new_memory( sizeof( struct substate ) );
  newsub->next = NULL;
  newsub->name = substate;
  newsub->once = once;
  newsub->state_case =
	mkcase ? get_state_case( substate, 0 ) : -1;
  return newsub;
}

static int needs_substate( int cmdtype ) {
  switch ( cmdtype ) {
	case CMDTYPE_TMC:
	case CMDTYPE_HOLD:
	case CMDTYPE_VHOLD:
	  return 1;
	case CMDTYPE_QSTR:
	case CMDTYPE_CMD:
	case CMDTYPE_VAL:
	  return 0;
	default:
	  compile_error( 4, "Unknown command type %d", cmdtype );
	  return 0;
  }
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
  struct cmddef *cmd, *ncmd;
  int i = 1;
  char buf[80];
  struct substate *substate;

  /* Skip over top commands */
  for (cmd = state->cmds; cmd != NULL && cmd->cmdtime < 0;
		cmd = cmd->next) {
	if ( cmd->cmdtype == CMDTYPE_TMC )
	  cmd->substate = new_substate( NULL, state->name, 0, 0 );
	else compile_error( 4,
		"Unexpected CMDTYPE %d at time < 0", cmd->cmdtype );
  }
  while ( cmd != 0 ) {
    /* look through the current time for TM commands */
	for ( ; cmd != 0; cmd = cmd->next )
	  if ( needs_substate( cmd->cmdtype ) ) break;

	/* look ahead to determine if this is the last state */
    if ( cmd != 0 ) {
	  if ( cmd->cmdtype == CMDTYPE_TMC ) {
		for ( ncmd = cmd->next;
			  ncmd != 0 && ncmd->cmdtime == cmd->cmdtime;
			  ncmd = ncmd->next );
		if ( ncmd == 0 )
		  sprintf( buf, "%s_end_", state->name );
		else sprintf( buf, "%s_%d_", state->name, i++ );
		substate = new_substate( ofp, buf, ncmd != 0, 1 );

		while ( cmd != ncmd ) {
		  if ( cmd->cmdtype == CMDTYPE_TMC )
			cmd->substate = substate;
		  cmd = cmd->next;
		}
	  } else {
		assert( cmd->cmdtype == CMDTYPE_HOLD || cmd->cmdtype == CMDTYPE_VHOLD );
		for ( ncmd = cmd; ncmd != 0; ncmd = ncmd->else_stat ) {
		  if ( needs_substate( ncmd->cmdtype ) ) {
			sprintf( buf, "%s_%d_", state->name, i++ );
			ncmd->substate = new_substate(ofp, buf, 0, 1 );
		  }
		}
		cmd = cmd->next;
	  }
	}
  }
}

static void end_substates( FILE *ofp ) {
  
  if ( curr_partition->partno >= 0 ) {
	char buf[40];
	/* create an idle substate for the partition */
	sprintf( buf, "part_%d_idle_", curr_partition->partno );
    curr_partition->idle_substate =
	  new_substate( ofp, buf, 0, 1 );
	end_state( ofp, buf );
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
		list_state(ofp, pi->u.state->name);
		if ( first_state == 0 )
		  first_state = pi->u.state->name;
		/* Make states available to slurp files: */
		get_state_case( pi->u.state->name, 1 );
		break;
	  case PRGTYPE_PARTITION:
		if ( curr_partition != 0 && state_listed )
		  curr_partition->partno = partno++;
		end_state(ofp, first_state);
		first_state = NULL;
		curr_partition = pi->u.partition;
		break;
	}
  }
  if ( curr_partition != 0 && state_listed )
	curr_partition->partno = partno++;
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
  assert( cmd->cmdtype == CMDTYPE_TMC );
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
  fprintf( ofp, "    tma_succeed( %d, %d );\n",
	curr_partition->partno, cmd->substate->state_case );
  fprintf( ofp, "    validate %s;\n  }\n}\n",
	curr_partition->idle_substate->name );
}

static void output_vhold( FILE *ofp, struct cmddef *cmd ) {
  output_dependency( ofp, cmd->substate, cmd->cmdtext );
  fprintf( ofp, "{\n  tma_succeed( %d, %d );\n",
	curr_partition->partno, cmd->substate->state_case );
  fprintf( ofp, "  validate %s;\n}\n",
	curr_partition->idle_substate->name );
}

void output_cmd_code( FILE *ofp, struct cmddef *cmd ) {
  int successor;
  struct cmddef *cmd1;
  
  assert( cmd != 0 );
  switch ( cmd->cmdtype ) {
	case CMDTYPE_CMD:
	  fprintf( ofp, "%8ld, \">%s\\n\",\n", cmd->cmdtime, cmd->cmdtext );
	  break;
	case CMDTYPE_VAL:
	  fprintf( ofp, "%8ld, \"#%d\", /* %s */\n", cmd->cmdtime,
		get_state_case( cmd->cmdtext, 0 ), cmd->cmdtext );
	  break;
	case CMDTYPE_QSTR:
	  fprintf( ofp, "%8ld, \"\\%s,\n", cmd->cmdtime, cmd->cmdtext );
	  break;
	case CMDTYPE_HOLD:
	case CMDTYPE_VHOLD:
	  successor = 1;
	  assert( cmd->substate != 0 );
	  for ( cmd1 = cmd->else_stat;
			cmd1 != 0;
			cmd1 = cmd1->else_stat ) {
		successor++;
	  }
	  fprintf( ofp, "%8ld, \"?%d,%ld,%d\", /* %s */\n", 
		cmd->cmdtime, successor, cmd->timeout,
		cmd->substate->state_case, cmd->substate->name );
	  break;
	case CMDTYPE_TMC:
	  assert( cmd->substate != 0 );
	  fprintf( ofp, "%8ld, \"#%d\", /* %s */\n", cmd->cmdtime,
		cmd->substate->state_case, cmd->substate->name );
	  break;
	default:
	  compile_error( 4,
		"Unexpected cmdtype %d in output_cmd_code",
		cmd->cmdtype );
  }
}

#define N_CMD_LIST_TYPES 3
static int type_list[N_CMD_LIST_TYPES] = {
  CMDTYPE_CMD, CMDTYPE_VAL, CMDTYPE_QSTR
};
/* Output one top-level state */
static void output_state(FILE *ofp, struct stdef *state ) {
  struct cmddef *cmd, *cmd0, *cmd1, *ncmd;
  long int t0;
  int subcase, prev_subcase, cti;
  char *substatename;

  /* Skip the T=-1 conditions */
  for (cmd = state->cmds; cmd != 0 && cmd->cmdtime < 0;
					cmd = cmd->next) {
	assert(cmd->cmdtype == CMDTYPE_TMC);
  }

  /* Define the state's command structure */
  fprintf( ofp, "%%{\n" );
  
  /* first print the commands */
  fprintf( ofp, "  tma_state %s_cmds_[] = {\n", state->name );
  t0 = 0;
  subcase = prev_subcase = -1;
  while ( cmd != 0 ) {
	for ( ncmd = cmd; ncmd != 0 && ncmd->cmdtime == cmd->cmdtime;
		  ncmd = ncmd->next ) {
	  if ( ncmd == 0 || ncmd->cmdtime != cmd->cmdtime ) break;
	  if ( ncmd->cmdtype == CMDTYPE_HOLD ||
			ncmd->cmdtype == CMDTYPE_VHOLD ) {
		ncmd = ncmd->next;
		break;
	  }
	}
	for ( cti = 0; cti < N_CMD_LIST_TYPES; cti++ ) {
	  for ( cmd0 = cmd; cmd0 != ncmd; cmd0 = cmd0->next ) {
		if ( cmd0->cmdtype == type_list[cti] )
		  output_cmd_code( ofp, cmd0 );
	  }
	}
	subcase = -1;
	substatename = "";
	for ( cmd0 = cmd; cmd0 != ncmd; cmd0 = cmd0->next ) {
	  if ( cmd0->cmdtype == CMDTYPE_TMC ) break;
	}
	if ( cmd0 != ncmd ) {
	  assert( cmd0->substate != 0 );
	  subcase = cmd0->substate->state_case;
	  substatename = cmd0->substate->name;
	  assert( subcase != -1 );
	  assert( subcase != prev_subcase );
	  fprintf( ofp, "%8ld, \"#%d\", /* %s */\n", cmd->cmdtime,
				subcase, substatename );
	  prev_subcase = subcase;
	}
	
	/* Now the conditionals */
	for ( cmd0 = cmd; cmd0 != ncmd; cmd0 = cmd0->next ) {
	  if ( cmd0->cmdtype == CMDTYPE_HOLD ||
		  cmd0->cmdtype == CMDTYPE_VHOLD )
		break;
	}
	if ( cmd0 != ncmd ) {
	  for ( cmd1 = cmd0; cmd1 != 0; cmd1 = cmd1->else_stat ) {
		output_cmd_code( ofp, cmd1 );
	  }
	} else {
	  if ( subcase == -1 ) {
		assert( curr_partition != 0 );
		assert( curr_partition->idle_substate != 0 );
		subcase = curr_partition->idle_substate->state_case;
		substatename = curr_partition->idle_substate->name;
		assert( subcase != -1 );
	  }
	  if ( subcase != prev_subcase ) {
		fprintf( ofp, "%8ld, \"#%d\", /* %s */\n", cmd->cmdtime,
				  subcase, substatename );
		prev_subcase = subcase;
	  }
	}
	cmd = ncmd;
  }
  fprintf( ofp, "%8ld, NULL\n  };\n", -1L );
  
  if (state->filename != 0 ) {
	fprintf( ofp, "  tma_ifile %s_file_ = {\n", state->name );
	fprintf( ofp, "    %d, %s, \"%s\", NULL, %s_cmds_, -1\n  };\n",
	  curr_partition->partno, state->filename, state->name, 
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
	fprintf( ofp, "  tma_init_state( %d, %s_cmds_, \"%s\" );\n",
			  curr_partition->partno, state->name, state->name );
  }
  fprintf( ofp, "  Validate %s;\n}\n", 
			curr_partition->idle_substate->name );

  /* output the substate commands */
  for ( cmd = state->cmds; cmd != 0; cmd = cmd->next ) {
	for ( cmd0 = cmd; cmd0 != 0; cmd0 = cmd0->else_stat ) {
	  switch ( cmd0->cmdtype ) {
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
  fprintf( ofp, "\t  case 0: break;\n" );
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
		fprintf( ofp, "    \"%s\", \"#%d\",\n",
		  subs->statename, subs->statecase );
	  }
	}
	fprintf( ofp, "    NULL, 0\n  };\n%%}\n" );
  }
}
