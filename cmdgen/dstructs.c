/* dstructs.c defines routines for creating and manipulating
 * data structures for command parser generator.
 * $Log$
 * Revision 1.4  2004/10/08 17:07:11  nort
 * Mostly keyword differences
 *
 * Revision 1.3  1995/05/25  17:19:56  nort
 * Use standard nortlib compiler functions
 *
 * Revision 1.2  1993/05/18  13:09:13  nort
 * Client/Server Support
 *
 * Revision 1.1  1992/10/20  20:27:07  nort
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
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

struct nt_t *non_terms = NULL;
struct sub_t **rules = NULL;
static unsigned short max_rules = 0;
unsigned short n_rules = 0;
unsigned short n_nonterms = 0;
#define RULE_INCREMENT 100

void *new_memory(unsigned int size) {
  void *m;
  
  m = malloc(size);
  if (m == NULL) compile_error(3, "Memory Allocation Failure");
  return(m);
}

/* Finds or creates a new non_terminal. */
struct nt_t *non_terminal(char *name) {
  struct nt_t *nt, *ntp, *ntn;
  int c;
  
  for (ntp = NULL, nt = non_terms; nt != NULL; ntp = nt, nt = nt->next)
	if ((c = stricmp(name, nt->name)) <= 0) break;
  if (nt != NULL && c == 0) return(nt);
  ntn = new_memory(sizeof(struct nt_t));
  ntn->next = nt;
  ntn->name = strdup(name);
  ntn->number = n_nonterms+1;
  ntn->type = NULL;
  ntn->rules.first = ntn->rules.last = NULL;
  if (ntp == NULL) non_terms = ntn;
  else ntp->next = ntn;
  if (n_nonterms++ == 0) { /* create start rule */
	struct sub_t *sub;
	struct sub_item_t *si;
	
	sub = new_sub();
	assert(sub->rule_number == 0);
	si = sub->items.first = sub->items.last = new_sub_item(SI_NT);
	si->u.nt = ntn;
  }
  return(ntn);
}

/* Dummy non-terminals are given names beginning with '_'.
   These could later be changed to '&' if they are part of
   a Client reduction.
*/
void dmy_non_term(struct sub_t *sub) {
  static int dummy_num = 0;
  char dbuf[10];
  struct nt_t *nt;
  struct sub_item_t *nsi;
  
  assert(sub->action != NULL);
  dbuf[0] = '_';
  /* itoa(dummy_num++, dbuf+1, 10); */
  sprintf( dbuf+1, "%d", dummy_num++ );
  nt = non_terminal(dbuf);
  nt->rules.first = nt->rules.last = new_sub();
  nt->rules.last->action = sub->action;
  sub->action = NULL;
  nsi = new_sub_item(SI_NT);
  nsi->u.nt = nt;
  if (sub->items.last == NULL)
	sub->items.first = sub->items.last = nsi;
  else sub->items.last = sub->items.last->next = nsi;
}

struct sub_t *new_sub(void) {
  struct sub_t *ns;

  if (n_rules == max_rules) {
	max_rules += RULE_INCREMENT;
	if (n_rules) rules = realloc(rules, max_rules * sizeof(struct sub_t *));
	else rules = malloc(max_rules * sizeof(struct sub_t *));
	if (rules == NULL) compile_error(3, "Rule List Allocation Failure");
  }
  ns = new_memory(sizeof(struct sub_t));
  ns->next = NULL;
  ns->reduces = NULL;
  ns->action = NULL;
  ns->items.first = ns->items.last = NULL;
  ns->rule_number = n_rules;
  rules[n_rules++] = ns;
  return(ns);
}

struct sub_item_t *new_sub_item(unsigned type) {
  struct sub_item_t *nsi;
  
  nsi = new_memory(sizeof(struct sub_item_t));
  nsi->next = NULL;
  nsi->type = type;
  return(nsi);
}
