/* cmdgen.c contains the main program for the Command Parser Generator.
 * $Log$
 * Revision 1.9  2007/05/01 17:42:27  ntallen
 * Added %INTERFACE <name> spec
 *
 * Revision 1.8  2006/05/30 00:17:09  ntallen
 * Portability (to compile under cygwin)
 *
 * Revision 1.7  2004/10/08 17:07:08  nort
 * Mostly keyword differences
 *
 * Revision 1.6  1995/05/25  17:20:06  nort
 * Use standard nortlib compiler functions
 *
 * Revision 1.5  1993/05/19  20:32:55  nort
 * Changed version string so it can be picked out by ident
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include "cmdgen.h"
#include "compiler.h"
#include "nortlib.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

int (*nl_error)(int level, const char *s, ...) = compile_error;
static int verbose = 0;
static char data_file[FILENAME_MAX+1] = "";
static time_t time_of_day;

char *opt_string = OPT_COMPILER_INIT "d:V";

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
	  nl_error(4, "Unexpected shift/reduce!");
  }
}

void print_rule_pos(FILE *fp, unsigned int rnum, int pos) {
  struct sub_item_t *si;
  
  assert(rnum < n_rules);
  for (si = rules[rnum]->items.first; ; si = si->next) {
    if (pos-- == 0) fprintf(fp, " .");
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

void print_state(FILE *sfile, state *st) {
  rulelist *rl;
  termlist *tl;
  ntermlist *ntl;
  
  for (rl = st->rules; rl != NULL; rl = rl->next) {
	print_rule_pos(sfile, rl->rule_number, rl->position);
	fputc('\n', sfile);
  }
  if (st->def_action.flag) {
	fprintf(sfile, "Default Action: ");
	print_action(&st->def_action);
  } else switch (st->terminal_type) {
	case SI_WORD:
	  for (tl = st->terminals; tl != NULL; tl = tl->next) {
		print_word(sfile, tl->term);
		print_action(&tl->action);
	  }
	  break;
	case SI_VSPC:
	  print_word(sfile, st->terminals->term);
	  print_action(&st->terminals->action);
	  break;
	case SI_EOR: break;
	default:
	  nl_error(4, "Unexpected terminal_type %d", st->terminal_type);
  }
  for (ntl = st->non_terminals; ntl != NULL; ntl = ntl->next) {
	fprintf(sfile, " &%s: ", ntl->nt->name);
	print_action(&ntl->action);
  }
}

static void print_states(void) {
  int i;
  state *st;
  
  fprintf(vfile, "Total of %d states\n", n_states);
  for (i = 0; i < n_states; i++) {
	st = states[i];
	assert(st->state_number == i);
	fprintf(vfile, "State %d:\n", i);
	print_state(vfile, st);
	fputc('\n', vfile);
  }
}

static void output_version(void) {
  fprintf(ofile, "char ci_version[] = \"$CGID: ");
  if (output_filename != NULL) fprintf(ofile, "%s", output_filename);
  else fprintf(ofile, "\" __FILE__ \"");
  fprintf(ofile, ": %24.24s $\";\n", ctime( &time_of_day));
}

static void generate_output(void) {
  FILE *ofile_save;

  output_interfaces();
  fprintf(ofile, "typedef unsigned %s cg_token_type;\n",
		  n_states > 255 || max_tokens > 255 ? "short" : "char");
  fprintf(ofile, "typedef unsigned %s cg_nonterm_type;\n",
		  n_nonterms > 255 ? "short" : "char");
  Skel_copy(ofile, "typedefs", 1);
  output_version();
  output_vdefs();
  Skel_copy(ofile, "tstack", 1);
  if (data_file[0] != '\0') {
	ofile_save = ofile;
	ofile = fopen(data_file, "w");
	if (ofile == NULL)
	  nl_error(3, "Unable to open data structure file %s", data_file);
  }
  output_trie();
  output_prompts();
  output_shifts();
  output_states();
  if (data_file[0] != '\0') {
	fclose(ofile);
	ofile = ofile_save;
	fprintf(ofile, "#include \"%s\"  /* Data Structure Definitions */\n",
					data_file);
  }
  output_rules();
}

#ifdef __USAGE
%C	[options] [filename]
	-q           Print this message
	-o filename  Write output to this file
	-v           General verbosity flag
	-V           Include verbose information
	-d filename  Write data structures to this file
	-k           keep output file on error
	-w           treat warnings as errors

Input Syntax Notes:
  Variables can be specified using the following % escapes:

  escape  type                Default Prompt
  ---------------------------------------------------------------------
    %d    short  Enter Integer (Decimal: 123, Hex: 0x123F, Octal: 0123)
    %ld   long   Enter Integer (Decimal: 123, Hex: 0x123F, Octal: 0123)
    %x    short  Enter Hexadecimal Number
    %lx   long   Enter Hexadecimal Number
    %o    short  Enter Octal Number
    %lo   long   Enter Octal Number
    %f    float  Enter Floating Point Number
    %lf   double Enter Floating Point Number
    %w    char * Enter Word Terminated by <space>
    %s    char * Enter Word Terminated by <CR>
#endif

static void main_args(int argc, char **argv) {
  int c;

  opterr = 0;
  optind = 0;
  while ((c = getopt(argc, argv, opt_string)) != -1) {
	switch (c) {
	  case 'd':
		strncpy(data_file, optarg, FILENAME_MAX);
		data_file[FILENAME_MAX] = 0;
		break;
	  case 'V':
		verbose = 1;
		break;
	  case '?':
		nl_error(3, "Unknown command option -%c", optopt);
	}
  }
  compile_init_options(argc, argv, ".c");
}

int main(int argc, char **argv) {
  main_args(argc, argv);
  Skel_open("cmdgen.skel");
  time_of_day = time(NULL);
  fprintf(ofile, "/* cmdgen output.\n * %s */\n", ctime( &time_of_day));
  Skel_copy(ofile, "headers", 1);
  if (yyparse() != 0) nl_error(3, "Parsing failed\n");
  if (error_level == 0) {
	if (verbose) {
	  if (vfile == ofile) fprintf(ofile, "#ifdef __DEFINITIONS\n");
	  print_rules();
	}
	eval_states();
  }
  if (error_level == 0) {
	if (verbose) {
	  print_states();
	  if (vfile == ofile) fprintf(ofile, "#endif\n");
	}
	generate_output();
	Skel_copy(ofile, NULL, 1);
  }
  return(error_level);
}
