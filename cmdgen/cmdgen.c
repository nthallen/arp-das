/* cmdgen.c contains the main program for the Command Parser Generator.
 *
 * $Log$
 * Revision 1.1  1992/10/20  19:45:08  nort
 * Initial revision
 *
 * Revision 1.1  1992/07/09  18:36:44  nort
 * Initial revision
 *
 */
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include "cmdgen.h"
#include "nortlib.h"
static char rcsid[] = "$Id$";

int (*nl_error)(unsigned int level, char *s, ...) = app_error;

static void print_word(FILE *fp, struct sub_item_t *si) {
  switch (si->type) {
	case SI_NT: fprintf(fp, " &%s", si->u.nt->name); break;
	case SI_WORD:
	  if (si->u.text[0] == '\n') fprintf(fp, " *");
	  else fprintf(fp, " %s", si->u.text);
	  break;
	case SI_VSPC:
	  fprintf(fp, " %s", si->u.vspc.format);
	  if (si->u.vspc.prompt != NULL)
		fprintf(fp, " (%s)", si->u.vspc.prompt);
	  break;
  }
}

static void print_action(response *act) {
  switch (act->flag & RSP_SHIFT_REDUCE) {
	case RSP_SHIFT:
	  fprintf(vfile, " shift to state %d\n", act->value);
	  break;
	case RSP_REDUCE:
	  fprintf(vfile, " reduce via rule %d\n", act->value);
	  break;
	case RSP_SHIFT_REDUCE:
	  app_error(4, "Unexpected shift/reduce!");
  }
}

void print_rule_pos(FILE *fp, unsigned int rnum, int pos) {
  struct sub_item_t *si;
  
  assert(rnum < n_rules);
  for (si = rules[rnum]->items.first; ; si = si->next) {
    if (pos-- == 0) fprintf(vfile, " .");
	if (si == NULL) break;
    print_word(fp, si);
  }
}

static void print_rules(void) {
  int i;
  
  for (i = 1; i < n_rules; i++) {
	fprintf(vfile, "%2d: &%s :", i, rules[i]->reduces->name);
	print_rule_pos(vfile, i, -1);
	fputc('\n', vfile);
  }
}

void print_state(state *st) {
  rulelist *rl;
  termlist *tl;
  ntermlist *ntl;
  
  for (rl = st->rules; rl != NULL; rl = rl->next) {
	print_rule_pos(vfile, rl->rule_number, rl->position);
	fputc('\n', vfile);
  }
  if (st->def_action.flag) {
	fprintf(vfile, "Default Action: ");
	print_action(&st->def_action);
  } else switch (st->terminal_type) {
	case SI_WORD:
	  for (tl = st->terminals; tl != NULL; tl = tl->next) {
		print_word(vfile, tl->term);
		print_action(&tl->action);
	  }
	  break;
	case SI_VSPC:
	  print_word(vfile, st->terminals->term);
	  print_action(&st->terminals->action);
	  break;
	case SI_EOR: break;
	default:
	  app_error(4, "Unexpected terminal_type %d", st->terminal_type);
  }
  for (ntl = st->non_terminals; ntl != NULL; ntl = ntl->next) {
	fprintf(vfile, " &%s: ", ntl->nt->name);
	print_action(&ntl->action);
  }
}

static void print_states(void) {
  int i;
  state *st;
  
  for (i = 0; i < n_states; i++) {
	st = states[i];
	assert(st->state_number == i);
	fprintf(vfile, "State %d:\n", i);
	print_state(st);
	fputc('\n', vfile);
  }
}

static void generate_output(void) {
  fprintf(ofile, "typedef unsigned %s cg_token_type;\n",
		  n_states > 255 || max_tokens > 255 ? "short" : "char");
  fprintf(ofile, "typedef unsigned %s cg_nonterm_type;\n",
		  n_nonterms > 255 ? "short" : "char");
  output_vdefs();
  output_trie();
  output_rules();
  output_prompts();
  output_shifts();
  output_states();
}

int main(void) {
  time_t time_of_day;

  Skel_open("cmdgen.skel");
  time_of_day = time(NULL);
  fprintf(ofile, "/* cmdgen output.\n * %s */\n", ctime( &time_of_day));
  Skel_copy(ofile, "headers", 1);
  if (yyparse() == 0) {
	if (vfile == ofile) fprintf(ofile, "#ifdef __DEFINITIONS\n");
	print_rules();
	eval_states();
	print_states();
	if (vfile == ofile) fprintf(ofile, "#endif\n");
	generate_output();
	Skel_copy(ofile, NULL, 1);
  } else fprintf(efile, "Parsing failed\n");
  return(0);
}
