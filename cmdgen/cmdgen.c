/* cmdgen.c contains the main program for the Command Parser Generator.
 *
 * $Log$
 * Revision 1.3  1992/10/27  08:38:20  nort
 * Added command line options
 *
 * Revision 1.2  1992/10/20  20:27:07  nort
 * Added IDs
 *
 * Revision 1.1  1992/10/20  19:45:08  nort
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
#include <time.h>
#include <unistd.h>
#include "cmdgen.h"
#include "nortlib.h"
static char rcsid[] = "$Id$";

int (*nl_error)(unsigned int level, char *s, ...) = app_error;
static int verbose = 0;
FILE *ofile = NULL;
static char data_file[FILENAME_MAX+1] = "";
static time_t time_of_day;

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
  
  fprintf(vfile, "Total of %d states\n", n_states);
  for (i = 0; i < n_states; i++) {
	st = states[i];
	assert(st->state_number == i);
	fprintf(vfile, "State %d:\n", i);
	print_state(st);
	fputc('\n', vfile);
  }
}

static void output_version(void) {
  fprintf(ofile, "char ci_version[] = __FILE__ \": %24.24s\";\n",
		  ctime( &time_of_day));
}

static void generate_output(void) {
  FILE *ofile_save;

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
	  app_error(3, "Unable to open data structure file %s", data_file);
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
	-h           Print this message
	-o filename  Write output to this file
	-V           Include verbose information
	-d filename  Write data structures to this file
#endif

static void main_args(int argc, char **argv) {
  int c;
  extern FILE *yyin;

  opterr = 0;
  while ((c = getopt(argc, argv, "ho:Vd:")) != -1) {
	switch (c) {
	  case 'o':
		ofile = fopen(optarg, "w");
		if (ofile == NULL)
		  app_error(3, "Unable to open output file %s", optarg);
		break;
	  case 'd':
		strncpy(data_file, optarg, FILENAME_MAX);
		data_file[FILENAME_MAX] = 0;
		break;
	  case 'V':
		verbose = 1;
		break;
	  case 'h':
		print_usage(argv);
		exit(0);
	  case '?':
		app_error(3, "Unknown command option -%c", optopt);
	}
  }
  if (optind < argc) {
	yyin = fopen(argv[optind], "r");
	if (yyin == NULL)
	  app_error(3, "Unable to open input file %s", argv[optind]);
  } else yyin = stdin;
  if (ofile == NULL) ofile = stdout;
}

int main(int argc, char **argv) {
  main_args(argc, argv);
  Skel_open("cmdgen.skel");
  time_of_day = time(NULL);
  fprintf(ofile, "/* cmdgen output.\n * %s */\n", ctime( &time_of_day));
  Skel_copy(ofile, "headers", 1);
  if (yyparse() == 0) {
	if (verbose) {
	  if (vfile == ofile) fprintf(ofile, "#ifdef __DEFINITIONS\n");
	  print_rules();
	}
	eval_states();
	if (verbose) {
	  print_states();
	  if (vfile == ofile) fprintf(ofile, "#endif\n");
	}
	generate_output();
	Skel_copy(ofile, NULL, 1);
  } else fprintf(efile, "Parsing failed\n");
  return(0);
}
