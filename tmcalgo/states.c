/* states.c
 * $Log$
 */
#include <string.h>
#include <assert.h>
#include "nortlib.h"
#include "yytype.h"
#include "memlib.h"

/* Check the new name against the list of names already defined */
static struct nmlst {
  struct nmlst *next;
  char *name;
} *namelist = NULL;

char *new_state_name(char *name) {
  struct nmlst *nl;

  for (nl = namelist; nl != NULL; nl = nl->next) {
	if (stricmp(nl->name, name) == 0) {
	  nl_error(2, "Duplicate State Name");
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
static int started = 0, line_length = 0, word_length;

/* list_state adds one state name to the current state
   declaration, taking into account line lengths, etc.
*/
static void list_state(FILE *ofp, const char *name) {
  if (!started) {
	fprintf(ofp, "state (");
	line_length = LINE_LENGTH_LIMIT - 7;
  } else {
	fputc(',', ofp);
	line_length--;
  }
  word_length = strlen(name);
  if (started) word_length++;
  line_length -= word_length;
  if (line_length < 0) {
	fprintf(ofp, "\n");
	line_length = LINE_LENGTH_LIMIT - word_length;
  }
  if (started) fputc(' ', ofp);
  else started = 1;
  fprintf(ofp, "%s", name);
}

/* end_state() ends the current state declaration (if one
   has in fact been started).
*/
static void end_state(FILE *ofp) {
  if (started) {
	fprintf(ofp, ");\n");
	started = 0;
  }
}
  
/* list_substates() outputs sub-state declarations for
   one state. We must have a State_0_, even
   if the state is otherwise empty. State_0_ will call
   new_time(partition, 0); to clear the "next cmd time"
*/
static void list_substates(FILE *ofp, struct stdef *state) {
  struct cmddef *cmd;
  long int t0;
  int i;
  char buf[30];

  t0 = 0;
  i = 0;
  for (cmd = state->cmds; cmd != NULL && cmd->cmdtime < 0;
		cmd = cmd->next);
  do {
	if (i > 0) t0 = cmd->cmdtime;
	sprintf(buf, "%s_%d_", state->name, i++);
	list_state(ofp, buf);
	while (cmd != NULL && cmd->cmdtime == t0)
	  cmd = cmd->next;
  } while (cmd != NULL);
}

/* List States outputs top-level state () declarations
   and sub-state declarations for each partition.
*/
void list_states(FILE *ofp) {
  struct prg *pi;

  for (pi = program; pi != NULL; pi = pi->next) {
	switch (pi->type) {
	  case PRGTYPE_STATE: list_state(ofp, pi->u.state->name); break;
	  case PRGTYPE_PARTITION: end_state(ofp); break;
	}
  }
  end_state(ofp);
  
  /* now list the substates for each partition */
  for (pi = program; pi != NULL; pi = pi->next) {
	switch (pi->type) {
	  case PRGTYPE_STATE: list_substates(ofp, pi->u.state); break;
	  case PRGTYPE_PARTITION: end_state(ofp); break;
	}
  }
  end_state(ofp);
}

/* Output one top-level state */
static void output_state(FILE *ofp, struct stdef *state, int partition) {
  struct cmddef *cmd, *cmd0;
  long int t0;
  int i;
  char *next_cmd;
  
  /* Display the T=-1 conditions */
  for (cmd = state->cmds; cmd != NULL && cmd->cmdtime < 0;
					cmd = cmd->next) {
	assert(cmd->cmdflags & CMDFLAGS_TMC);
	fprintf(ofp, "depending on (%s) {\n%s\n}\n",
			state->name, cmd->cmdtext);
  }

  /* Now define the main state */
  fprintf(ofp, "depending on (%s once) {\n", state->name);
  fprintf(ofp, "  tma_new_state(%d, \"%s\");\n", partition, state->name);
  fprintf(ofp, "  validate %s_0_;\n}\n", state->name);

  /* Now define State_i_ */
  t0 = 0;
  i = 0;
  do {
	fprintf(ofp, "depending on (%s_%d_ once) {\n", state->name, i);
	for ( ; cmd != NULL && cmd->cmdtime == t0 &&
		    (cmd->cmdflags & CMDFLAGS_TMC) == 0;
			cmd = cmd->next)
	  fprintf(ofp, "  ci_sendcmd(\"%s\\n\", tma_test);\n", cmd->cmdtext);

	/* Now hold our position but look ahead for the next time
	   and the next command.
	*/
	cmd0 = cmd;
	while (cmd != NULL && cmd->cmdtime == t0) cmd = cmd->next;
	if (cmd != NULL) {
	  t0 = cmd->cmdtime;
	  if (cmd->cmdflags & CMDFLAGS_TMC) next_cmd = "";
	  else next_cmd = cmd->cmdtext;
	} else {
	  t0 = 0;
	  next_cmd = "";
	}
	fprintf(ofp, "  tma_new_time(%d, %ld, \"%s\");\n}\n",
			partition, t0, next_cmd);

	/* Now display individual conditions for this substate */
	for (; cmd0 != cmd; cmd0 = cmd0->next) {
	  assert(cmd0 != NULL && (cmd0->cmdflags & CMDFLAGS_TMC));
	  fprintf(ofp, "depending on (%s_%d_) {\n%s\n}\n",
		state->name, i, cmd0->cmdtext);
	}
	
	/* Output a tma_time_check if there are more commands */
	if (t0 > 0) {
	  fprintf(ofp, "depending on (%s_%d_, 1 Hz) {\n", state->name, i);
	  fprintf(ofp, "  if (tma_time_check(%d))\n", partition);
	  fprintf(ofp, "    validate %s_%d_;\n", state->name, ++i);
	  fprintf(ofp, "}\n");
	} else assert(cmd == NULL);
  } while (cmd != NULL);
}

/* Output the actual commands */
void output_states(FILE *ofp) {
  struct prg *pi;
  int partition = 0, saw_state = 0;

  for (pi = program; pi != NULL; pi = pi->next) {
	switch (pi->type) {
	  case PRGTYPE_STATE:
		output_state(ofp, pi->u.state, partition);
		saw_state = 1;
		break;
	  case PRGTYPE_TMCCMD:
		fprintf(ofp, "%s\n", pi->u.tmccmd);
		break;
	  case PRGTYPE_PARTITION:
		if (saw_state) {
		  partition++;
		  saw_state = 0;
		}
		break;
	}
  }
}
