/* prompts.c
 *
 * $Log$
 */
#include <stdio.h>
#include <assert.h>
#include "cmdgen.h"

static int n_prompts = 0;
static void pprompt(int more, char *text) {
  if (n_prompts > 0) putc(',', ofile);
  fprintf(ofile, "\n  %2d, \"%s\"", more, text);
  n_prompts++;
}

static void gen_tprompts(termlist *tl) {
  char buf[81], *this;
  int n_rows = 0, n_tls_this, n_chars_this, more, bp = 0;
  
  for (n_tls_this = 0, more = 0; tl != NULL; ) {
	assert(tl->term != NULL);
	assert(tl->term->type == SI_WORD);
	assert(tl->term->u.text != NULL);
	n_tls_this++;
	if (n_tls_this == 8 && tl->next != NULL) {
	  this = "<More>";
	  more = 1;
	} else {
	  this = tl->term->u.text;
	  tl = tl->next;
	}
	if (*this == '\n') this = "<CR>";
	for (n_chars_this = 0; n_chars_this < 9 && *this != '\0';
		 n_chars_this++) {
	  buf[bp++] = *this++;
	}
	if (n_tls_this < 8 && tl != NULL)
	  while (n_chars_this++ < 10) buf[bp++] = ' ';
	else {
	  buf[bp] = '\0';
	  if (more && tl == NULL) more = -n_rows;
	  pprompt(more, buf);
	  n_rows++;
	}
  }
}

void output_prompts(void) {
  int i;

  fprintf(ofile, "prompt_type prompts[] = {");
  for (i = 0; i < n_states; i++) {
	if (states[i]->terminal_type == SI_WORD) {
	  states[i]->prompt_offset = n_prompts;
	  gen_tprompts(states[i]->terminals);
	} else if (states[i]->terminal_type == SI_VSPC) {
	  assert(states[i]->terminals != NULL);
	  assert(states[i]->terminals->term != NULL);
	  assert(states[i]->terminals->term->type == SI_VSPC);
	  states[i]->prompt_offset = n_prompts;
	  pprompt(0, states[i]->terminals->term->u.vspc.prompt);
	}
  }
  fprintf(ofile, "\n};\n");
}
