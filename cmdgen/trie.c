/* trie.c Defines grand trie structure in output file.
 *
 * $Log$
 * Revision 1.4  2004/10/08 17:07:17  nort
 * Mostly keyword differences
 *
 * Revision 1.3  1995/05/25  17:21:02  nort
 * Use standard nortlib compiler functions
 *
 * Revision 1.2  1992/10/20  20:29:39  nort
 * Corrected IDs
 *
 * Revision 1.1  1992/10/20  20:28:46  nort
 * Initial revision
 *
 * Revision 1.1  1992/07/09  18:36:44  nort
 * Initial revision
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include "cmdgen.h"
#include "compiler.h"
#include "nortlib.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

static unsigned short trie_offset = 0;

typedef struct trnd {
  struct trnd *child;
  struct trnd *sib;
  unsigned char code;
  unsigned short number;
  unsigned short shift;
} tnode;

static tnode *new_tnode(unsigned char code) {
  tnode *tn;

  tn = new_memory(sizeof(tnode));
  tn->child = tn->sib = NULL;
  tn->code = code;
  tn->number = 0;
  tn->shift = 0;
  return(tn);
}

/* add_to_trie will assume the input is already in alphabetical order */
static void add_to_trie(tnode *tn, char *s, unsigned short shift) {
  tnode *tnc, *tncp;
  int c;

  if (*s != 0 && *s != '\n' && !isprint(*s))
    compile_error(2, "add_to_trie character 0x%2X", *s);
  c = tolower(*s);
  for (tncp = NULL, tnc = tn->child; tnc != NULL;
	   tncp = tnc, tnc = tnc->sib)
	if (tolower(tnc->code) == tolower(*s)) break;
  if (tnc != NULL) {
	if (c == 0)
	  compile_error(4, "Duplicate trie entry!");
  } else {
	tnc = new_tnode(*s);
	if (tncp != NULL) tncp->sib = tnc;
	else tn->child = tnc;
	if (c == 0) {
	  tnc->shift = shift;
	  return;
	}
  }
  add_to_trie(tnc, s+1, shift);
}

/* Returns 1 if we did something, zero otherwise */
static int number_trie(tnode *tn, int depth) {
  tnode *tnp;
  int result = 0;

  if (depth == 0) {
	tn->number = trie_offset++;
	result = 1;
  } else for (tnp = tn->child; tnp != NULL; tnp = tnp->sib)
	if (number_trie(tnp, depth-1)) result = 1;
  return(result);
}

/* Returns 1 if we did something, zero otherwise */
static int output_tnodes(tnode *tn, int depth, int from) {
  tnode *tnp;
  int result = 0;
  int code, next, prev;

  if (depth == 0) {
	if (tn->number != 0) putc(',', ofile);
	if (tn->child == NULL) {
	  assert(tn->code == 0);
	  next = tn->shift;
	} else next = tn->child->number - tn->number;
	if (tn->code == 0x1) prev = 0;
	else prev = tn->number - from;
	code = tn->code;
	if (tn->sib == NULL && code != 1) code |= 0x80;
	fprintf(ofile, "\n  { 0x%02X", code);
	if (isprint(code &= 0x7F)) fprintf(ofile, " /* '%c' */", code);
	else fprintf(ofile, "          ");
	fprintf(ofile, ", %3d, %3d }", next, prev);
	fflush(ofile);
	result = 1;
  } else for (tnp = tn->child; tnp != NULL; tnp = tnp->sib) {
	if (output_tnodes(tnp, depth-1, tn->number))
	  result = 1;
  }
  return(result);
}

static void free_trie(tnode *tn) {
  tnode *tnp, *ntnp;
  
  for (tnp = tn->child; tnp != NULL; tnp = ntnp) {
	ntnp = tnp->sib;
	free_trie(tnp);
  }
  free_memory(tn);
}

static void gen_trie(termlist *terminals) {
  tnode *tn;
  int depth;
  
  /* build the trie */
  tn = new_tnode(0x1);
  for (; terminals != NULL; terminals = terminals->next) {
	assert(terminals->term->type == SI_WORD);
	assert(terminals->action.flag == RSP_SHIFT);
    add_to_trie(tn, terminals->term->u.text, terminals->action.value);
  }
  
  /* number trie breadth-first */
  for (depth = 0; number_trie(tn, depth); depth++);
  
  /* output trie breadth-first */
  for (depth = 0; output_tnodes(tn, depth, 0); depth++);
  
  /* free the trie */
  free_trie(tn);
}

void output_trie(void) {
  int i;
  
  fprintf(ofile, "trie_type trie[] = {");
  for (i = 0; i < n_states; i++) {
	if (states[i]->terminal_type == SI_WORD) {
	  states[i]->term_offset = trie_offset;
	  gen_trie(states[i]->terminals);
	}
  }
  fprintf(ofile, "\n};\n");
};
