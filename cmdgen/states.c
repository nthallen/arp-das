/* states.c
 *
 * $Log$
 * Revision 1.5  2004/10/08 17:07:14  nort
 * Mostly keyword differences
 *
 * Revision 1.4  1995/05/25  17:20:41  nort
 * Use standard nortlib compiler functions
 *
 * Revision 1.3  1993/07/09  19:48:42  nort
 * Filled out reduce_state() to make nesting possible.
 *
 * Revision 1.2  1992/10/27  08:38:20  nort
 * Removed illegal output
 *
 * Revision 1.1  1992/10/20  20:25:37  nort
 * Initial revision
 *
 * Revision 1.1  1992/07/09  18:36:44  nort
 * Initial revision
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "cmdgen.h"
#include "compiler.h"
#include "nortlib.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

state **states = NULL;
unsigned short n_states = 0;
unsigned short max_tokens = 0;
#define STATE_INCREMENT 100

state *new_state(void) {
  static unsigned short max_states = 0;
  state *ns;
  
  if (n_states == max_states) {
	max_states += STATE_INCREMENT;
	if (n_states) states = realloc(states, max_states * sizeof(state *));
	else states = malloc(max_states * sizeof(state *));
	if (states == NULL) compile_error(4, "State List Allocation Failure");
  }
  ns = states[n_states] = new_memory(sizeof(state));
  ns->state_number = n_states++;
  ns->terminal_type = 0;
  ns->n_terminals = 0;
  ns->rules = NULL;
  ns->def_action.flag = 0;
  ns->terminals = NULL;
  ns->non_terminals = NULL;
  return(ns);
}

/* Adds the specified rule number and position to the specified state
   If it isn't already listed, add to appropriate additional list.
   If non-terminal, expand recursively.
*/
void add_rule_pos(state *st, unsigned short rnum, unsigned short pos) {
  rulelist *rl, *rlp, *nrl;
  struct sub_t *sub;
  struct sub_item_t *si;
  unsigned char item_type;
  
  for (rlp = NULL, rl = st->rules; rl != NULL; rlp = rl, rl = rl->next) {
	if (rnum == rl->rule_number	&& pos == rl->position)
	  return; /* It's already here */
	else if (rnum < rl->rule_number
			 || (rnum == rl->rule_number
				 && pos < rl->position))
	  break;
  }
  nrl = new_memory(sizeof(rulelist));
  nrl->next = rl;
  nrl->rule_number = rnum;
  nrl->position = pos;
  for (si = rules[rnum]->items.first; si != NULL && pos > 0;
	   si = si->next, pos--);
  assert(pos == 0);
  nrl->si = si;
  if (rlp == NULL) st->rules = nrl;
  else rlp->next = nrl;
  if (si == NULL) item_type = SI_EOR;
  else item_type = si->type;
  if (item_type == SI_NT) {
	ntermlist *ntl, *ntlp, *nntl;
	
	assert(si->u.nt != NULL);
	/* Add to the list of non-terminals */
	for (ntlp = NULL, ntl = st->non_terminals;
		 ntl != NULL;
		 ntlp = ntl, ntl = ntl->next) {
	  if (si->u.nt->number <= ntl->nt->number) break;
	}
	if (ntl == NULL || si->u.nt->number != ntl->nt->number) {
	  nntl = new_memory(sizeof(ntermlist));
	  nntl->next = ntl;
	  nntl->nt = si->u.nt;
	  nntl->action.flag = 0;
	  nntl->action.value = 0;
	  if (ntlp == NULL) st->non_terminals = nntl;
	  else ntlp->next = nntl;
	}
	
	/* Then expand */
	for (sub = si->u.nt->rules.first; sub != NULL; sub = sub->next)
	  add_rule_pos(st, sub->rule_number, 0);
  } else {
	termlist *tl, *tlp, *ntl;
	int c;
	  
	if (st->terminal_type == 0) st->terminal_type = item_type;
	else if (st->terminal_type != item_type) {
	  if (st->terminal_type == SI_EOR || item_type == SI_EOR)
		compile_error(2, "Shift/Reduce conflict:");
	  else compile_error(2, "Variable/Keyword conflict:");
	  print_state(stderr, st);
	  fprintf(stderr, "\n");
	  compile_error(3, "Cannot Continue");
	}
	/* terminal types are SI_VSPC, SI_WORD and SI_EOR
	   If SI_WORD, we need to sort the terminals.
	   otherwise, there should only be one.
	*/
	tlp = NULL;
	tl = st->terminals;
	c = 1;
	if (item_type == SI_WORD) {
	  for (; tl != NULL; tlp = tl, tl = tl->next) {
		c = stricmp(si->u.text, tl->term->u.text);
		if (c <= 0) break;
	  }
    } else if (tl != NULL) {
	  compile_error(2, "Variable/Variable or EOR/EOR conflict:");
	  print_state(stderr, st);
	  fprintf(stderr, "\n");
	  compile_error(3, "Cannot Continue");
	}
	if (c != 0) {
	  ntl = new_memory(sizeof(termlist));
	  ntl->next = tl;
	  ntl->term = si;
	  if (item_type == SI_EOR) {
		ntl->action.flag = RSP_REDUCE;
		ntl->action.value = rnum;
	  } else {
		ntl->action.flag = 0;
		ntl->action.value = 0;
	  }
	  if (tlp == NULL) st->terminals = ntl;
	  else tlp->next = ntl;
	  if (++st->n_terminals > max_tokens)
		max_tokens = st->n_terminals;
	}
  }
}

/* reduce_state will check to see if the new state is identical
   to a previously defined state. The states are identical if
   they have the same set of rule/pos elements.
*/
state *reduce_state(state *st) {
  int i;
  rulelist *rla, *rlb;
  state *sta;

  for (i = 0; i < st->state_number; i++) {
	sta = states[i];
	for (rla = sta->rules, rlb = st->rules;
		rla != NULL && rlb != NULL; rla = rla->next, rlb = rlb->next) {
	  if (rla->rule_number != rlb->rule_number
		  || rla->position != rlb->position)
		break;
	}
	if (rla == rlb) { /* both must be NULL */
	  /* states are identical: release new state */
	  assert(st->state_number == n_states-1);
	  n_states--;
	  /* now free the rules, terminals, and non_terminals, then st */
	  for (rla = st->rules; rla != NULL; rla = rlb) {
		rlb = rla->next;
		free_memory(rla);
	  }
	  { termlist *tla, *tlb;
		for (tla = st->terminals; tla != NULL; tla = tlb) {
		  tlb = tla->next;
		  free_memory(tla);
		}
	  }
	  { ntermlist *nta, *ntb;
		for (nta = st->non_terminals; nta != NULL; nta = ntb) {
		  ntb = nta->next;
		  free_memory(nta);
		}
	  }
	  free_memory(st);
	  return(sta);
	}
  }
  return(st);
}

void eval_states(void) {
  state *st, *nst;
  rulelist *rl;
  termlist *tl;
  ntermlist *ntl;
  unsigned int snum;
  
  /* create state 0 using rule number zero's non-terminal as the
     start symbol */
  if (n_rules == 0) return;
  st = new_state();
  add_rule_pos(st, 0, 0);
  
  for (snum = 0; snum < n_states; snum++) {
	st = states[snum];
	
	/* If all terminal actions agree, set default action to reduce
	   For each terminal and non-terminal, generate a new state by
	   advancing by that element, adding each rule which shows that
	   element as next.
	*/
	if (st->terminal_type == SI_EOR) {
	  assert(st->terminals != NULL);
	  st->def_action = st->terminals->action;
	} else for (tl = st->terminals; tl != NULL; tl = tl->next) {
	  nst = new_state();
	  for (rl = st->rules; rl != NULL; rl = rl->next) {
		if (rl->si->type == tl->term->type
			&& (tl->term->type != SI_WORD
				|| stricmp(rl->si->u.text, tl->term->u.text) == 0))
		  add_rule_pos(nst, rl->rule_number, rl->position+1);
	  }
	  nst = reduce_state(nst);
	  tl->action.flag = RSP_SHIFT;
	  tl->action.value = nst->state_number;
	}
	for (ntl = st->non_terminals; ntl != NULL; ntl = ntl->next) {
	  /* <could check to see if we can actually get here>
	     If there is a default action and the reduced rule
		 has any sub items, we can't.
	  */
	  if (ntl->nt->rules.first == NULL)
		compile_error(2, "non-terminal %s is undefined", ntl->nt->name);
	  nst = new_state();
	  for (rl = st->rules; rl != NULL; rl = rl->next) {
		if (rl->si != NULL
			&& rl->si->type == SI_NT
			&& rl->si->u.nt->number == ntl->nt->number)
		  add_rule_pos(nst, rl->rule_number, rl->position+1);
	  }
	  nst = reduce_state(nst);
	  ntl->action.flag = RSP_SHIFT;
	  ntl->action.value = nst->state_number;
	}
  }
}

/* output_shifts outputs the non_terminals[] array.
   The nt member is the non_terminal's number.
   The shift member is the state number to which to shift.
   Non_terminal numbers are guaranteed to be greater
   than zero. The zero value is overloaded to serve two
   purposes: it indicates the last element of the list of
   non-terminals for a given state and also marks the
   shift for states which accept variable input.
*/
void output_shifts(void) {
  unsigned short snum, offset, ntnum, shift;
  int null_offset = -1;
  state *st;
  ntermlist *nt;

  fprintf(ofile, "shift_type non_terminals[] = {");
  offset = 0;
  for (snum = 0; snum < n_states; snum++) {
	st = states[snum];
	st->nonterm_offset = offset;
	nt = st->non_terminals;
	if (null_offset >= 0 && nt == NULL && st->terminal_type != SI_VSPC)
	  st->nonterm_offset = null_offset;
	else for (; ; nt = nt->next) {
	  if (nt == NULL) {
		ntnum = 0;
		if (st->terminal_type == SI_VSPC) {
		  assert(st->terminals != NULL);
		  assert(st->terminals->term != NULL);
		  assert(st->terminals->term->type == SI_VSPC);
		  assert(st->terminals->action.flag == RSP_SHIFT);
		  shift = st->terminals->action.value;
		} else {
		  shift = 0;
		  if (null_offset < 0) null_offset = offset;
		}
	  } else {
		assert(nt->nt != NULL);
		ntnum = nt->nt->number;
		assert(nt->action.flag == RSP_SHIFT);
		shift = nt->action.value;
	  }
	  if (offset != 0) putc(',', ofile);
	  fprintf(ofile, "\n  { %d, %d }", ntnum, shift);
	  offset++;
	  if (nt == NULL) break;
	}
  }
  fprintf(ofile, "\n};\n");
}

void output_states(void) {
  unsigned short snum;
  unsigned short nterm_offset;
  state *st;
  
  fprintf(ofile, "state_type states[] = {");
  for (snum = 0; snum < n_states; snum++) {
	if (snum > 0) putc(',', ofile);
	st = states[snum];
	nterm_offset = st->nonterm_offset;
	switch (st->terminal_type) {
      case SI_WORD:
		fprintf(ofile, "\n  { STFL_WORD, %d, %d, %d }", st->term_offset,
				st->prompt_offset, st->nonterm_offset);
		break;
      case SI_VSPC:
		fprintf(ofile, "\n  { STFL_VARIABLE, %s, %d, %d }",
				st->terminals->term->u.vspc.symbol,
				st->prompt_offset, st->nonterm_offset);
		break;
      case SI_EOR:
		assert(st->terminals != NULL);
		assert(st->terminals->term == NULL);
		fprintf(ofile, "\n  { STFL_REDUCE, %d, -1, %d }",
				st->terminals->action.value, st->nonterm_offset);
		break;
      case SI_NT:
	  default:
		compile_error(4, "Unexpected terminal type %d", st->terminal_type);
	}
  }
  fprintf(ofile, "\n};\n");
}
