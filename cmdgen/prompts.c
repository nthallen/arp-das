/* prompts.c
 *
 * $Log$
 * Revision 1.7  2009/10/01 19:54:53  ntallen
 * Cleanup output to quiet new compiler warnings
 *
 * Revision 1.6  2004/10/08 17:07:12  nort
 * Mostly keyword differences
 *
 * Revision 1.5  1995/05/25  17:21:13  nort
 * Use standard nortlib compiler functions
 *
 * Revision 1.4  1993/05/18  13:10:47  nort
 * NO_PROMPTS support
 *
 * Revision 1.3  1992/07/15  20:29:10  nort
 * Beta Release
 *
 * Revision 1.2  1992/07/10  19:31:47  nort
 * Added machine-dependent prompt texts
 * and consolidated common prompts.
 *
 * Revision 1.1  1992/07/09  18:36:44  nort
 * Initial revision
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "cmdgen.h"
#include "compiler.h"
#include "nortlib.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

#define PROMPT_ATTR 7

struct pidx {
  struct pidx *next;
  char *text;
};

static int n_unique = 0;
static struct pidx *ups;

static void output_pbyte(int c, char *t) {
  static int nchars = 0;
  int n;
  
  if (nchars == 0) {
	fprintf(ofile, "   ");
	nchars = 3;
  } else {
	putc(',', ofile);
	nchars++;
  }
  if (nchars > 70) {
	fprintf(ofile, "\n   ");
	nchars = 3;
  }
  if (t != NULL) fprintf(ofile, " %s%n", t, &n);
  else fprintf(ofile, " %d%n", c, &n);
  nchars += n;
}

static void output_ptext(void) {
  struct pidx *up, *nup;
  int nchars;
  char *s;
  
  fprintf(ofile, "#ifdef MACHINE_PROMPTS\n"
				 "  unsigned char prmt_text[] = {\n");
  for (up = ups; up != NULL; up = up->next) {
	for (nchars = 0, s = up->text; nchars < 80;
								nchars++) {
	  output_pbyte(*s == '\0' ? ' ' : *s++, NULL);
	  output_pbyte(0, "PROMPT_ATTR");
	}
  }
  fprintf(ofile, "\n  };\n#else\n  const char *prmt_text[] = {");
  for (up = ups; up != NULL; up = nup) {
	if (up != ups) putc(',', ofile);
	fprintf(ofile, "\n    \"%s\"", up->text);
	nup = up->next;
	free_memory(up->text);
	free_memory(up);
  }
  fprintf(ofile, "\n  };\n#endif\n");
  fprintf(ofile, "#endif /* NO_PROMPTS */\n");
}

static struct pidx *new_ups(char *text) {
  struct pidx *nups;
  
  nups = new_memory(sizeof(struct pidx));
  nups->text = strdup(text);
  nups->next = NULL;
  n_unique++;
  return(nups);
}

static int prompt_idx(char *text) {
  struct pidx *up, *nup;
  int cnt;
  
  if (n_unique == 0) {
	ups = new_ups(text);
  } else {
	for (cnt = 0, up = ups;
		 up != NULL;
		 cnt++, nup = up, up = up->next)
	  if (strcmp(text, up->text) == 0) return(cnt);
	nup->next = new_ups(text);
  }
  return(n_unique-1);
}

static int n_prompts = 0;
static void pprompt(int more, char *text) {
  if (n_prompts > 0) putc(',', ofile);
  fprintf(ofile, "\n  { %2d, PRMTOFST(%d) }", more, prompt_idx(text));
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
	  n_tls_this = 0;
	  bp = 0;
	  n_rows++;
	}
  }
}

void output_prompts(void) {
  int i;

  fprintf(ofile, "#ifndef NO_PROMPTS\n");
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
  output_ptext();
}
