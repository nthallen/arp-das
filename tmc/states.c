/* states.c functions for supporting state variables
 * $Log$
 * Revision 1.1  2008/07/03 15:11:07  ntallen
 * Copied from QNX4 version V1R9
 *
 * Revision 1.3  1995/10/18 02:05:07  nort
 * *** empty log message ***
 *
 * Revision 1.2  1993/09/27  19:40:45  nort
 * Cleanup, common compiler functions, output beautification.
 *
 * Revision 1.1  1993/05/21  19:46:26  nort
 */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "nortlib.h"
#include "rational.h"
#include "tmcstr.h"
#include "tmc.h"

static char rcsid[] =
      "$Id$";

static struct stateset *statesets = NULL;
static unsigned short n_sets = 0;

struct stateset *new_stateset(void) {
  struct stateset *set;
  char buf[20];
  
  set = new_memory(sizeof(struct stateset));
  set->next = statesets;
  statesets = set;
  sprintf(buf, "VSET_%d_", n_sets++);
  set->funcname = strdup(buf);
  set->states = NULL;
  set->n_states = 0;
  return(set);
}

void add_state(struct stateset *set, char *name) {
  struct statevar *stt;
  struct nm *nameref;

  assert(set != NULL && name != NULL);
  nameref = find_ref(name, 1);
  if (nameref->type != NMTYPE_UNDEFINED)
	compile_error(2, "Attempted redeclaration of name %s", name);
  else {
	/* Initialize the state */
	stt = new_memory(sizeof(struct statevar));
	stt->index = ++set->n_states;
	stt->funcname = set->funcname;
	nameref->type = NMTYPE_STATE;
	nameref->u.stdecl = stt;

	/* Initialize its validator */
	stt->vldtr = new_validator();
	stt->vldtr->flag = DCLF_VALIDATE | DCLF_TLSET;
	
	/* Link it into the set's namelist */
	set->states = newnamelist(set->states, nameref);
  }
}

static void print_cases(struct stateset *set, char *stname, int valid) {
  struct validator *vldtr;
  struct nmlst *nl;

  print_indent("switch (");
  fprintf(ofile, "%s) {", stname);
  adjust_indent(2);
  for (nl = set->states; nl != NULL; nl = nl->prev) {
	vldtr = nr_validator(nl->names);
	if (vldtr->val != NULL
		|| (valid && vldtr->valstat.first != NULL)) {
	  print_indent(NULL);
	  fprintf(ofile, "case %d: /* %s */", nl->names->u.stdecl->index,
						nl->names->name);
	  adjust_indent(2);
	  print_vldtr(vldtr, valid);
	  print_indent("break;");
	  adjust_indent(-2);
	}
  }
  adjust_indent(-2);
  print_indent("}");
  adjust_indent(0);
}

/* print_states prints the invalidation functions for each stateset */
void print_states(void) {
  struct stateset *set;
  
  adjust_indent(-80);
  for (set = statesets; set != NULL; set = set->next) {
	print_indent(NULL);
	fprintf(ofile, "static void %s(unsigned short newstate) {",
			set->funcname);
	adjust_indent(2);
	print_indent("static unsigned short oldstate;\n\n");
	print_cases(set, "oldstate", 0);
	print_indent("oldstate = newstate;");
	adjust_indent(0);
	print_cases(set, "newstate", 1);
	adjust_indent(-2);
	print_indent("}");
	adjust_indent(0);
  }
}

/* we can assume the state is being validated as opposed to invalidated,
   since the syntax enforces that.
 */
void print_st_valid(struct nm *nr) {
  struct statevar *sv;
  
  sv = nr->u.stdecl;
  print_indent(" ");
  fprintf(ofile, "%s(%d);", sv->funcname, sv->index);
}
