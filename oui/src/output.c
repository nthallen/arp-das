/* output.c contains output routines for oui
 * $Log$
 * Revision 1.6  2001/10/10 20:42:31  nort
 * Compiled under QNX6. Still need to configure with automake
 * and autoconf.
 *
 * Revision 1.5  2001/10/05 20:43:24  nort
 * Indentation
 *
 * Revision 1.4  1994/10/18  18:33:03  nort
 * Made a compromise on indentation
 *
 * Revision 1.3  1994/10/18  18:10:06  nort
 * Modified indentation to support R2
 *
 * Revision 1.2  1994/09/16  15:09:49  nort
 * Added sorting of sort options
 *
 * Revision 1.1  1994/09/15  19:45:34  nort
 * Initial revision
 *
 */
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include "nortlib.h"
#include "compiler.h"
#include "ouidefs.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

void output_comments(void) {
  llpkgleaf *p;

  fprintf(ofile, "/* OUI output from the following packages:\n");  
  for (p = global_defs.packages.first; p != NULL; p = p->next) {
	fprintf(ofile, "   %s\n", p->pkg->name);
  }
  fprintf(ofile, "*/\n");
}

void output_opt_string(void) {
  llpkgleaf *p;
  char *s;

  fprintf(ofile, "char *opt_string = \"");  
  for (p = global_defs.packages.first; p != NULL; p = p->next) {
	s = p->pkg->opt_string;
	if (s != 0) {
	  while (isspace(*s)) s++;
	  fprintf(ofile, "%s", s);
	}
  }
  fprintf(ofile, "\";\n");
}

static dump_llos( ll_of_str *ll, char *prefix ) {
  char *s;

  while ( s = llos_deq( ll ) ) {
	fprintf(ofile, "%s%s\n", prefix, s);
	free_memory(s);
  }
}

void output_defs(void) {
  llpkgleaf *p;
  
  for (p = global_defs.packages.first; p != NULL; p = p->next) {
	assert(p->pkg != 0);
	dump_llos( &p->pkg->defs, "" );
  }
}

static void output_switch(void) {
  llpkgleaf *p;

  fprintf(ofile, "%s", "\n"
	"  { int optltr;\n\n"
	"\toptind = OPTIND_RESET;\n"
	"\topterr = 0;\n"
	"\twhile ((optltr = getopt(argc, argv, opt_string)) != -1) {\n"
	"\t  switch (optltr) {\n");

  /* Dump the switch args */
  for (p = global_defs.packages.first; p != NULL; p = p->next)
	dump_llos( &p->pkg->switches, "\t\t" );

  fflush(ofile);

  fprintf(ofile, "%s",
	"\t\t  case '?':\n"
	"\t\t\tnl_error(3, \"Unrecognized Option -%c\", optopt);\n"
	"\t\t  default:\n"
	"\t\t\tbreak;\n"
	"\t  }\n"
	"\t}\n");

  fflush(ofile);

  if (arg_needed) {
	fprintf(ofile, "%s",
	  "\tfor (; optind < argc; optind++) {\n"
	  "\t  optarg = argv[optind];\n");

	/* Now the arg args */
	for (p = global_defs.packages.first; p != NULL; p = p->next)
	  dump_llos( &p->pkg->arg, "\t" );

	fprintf(ofile, "\t}\n");
  }
  fprintf(ofile, "  }\n");
}

void output_inits(void) {
  llpkgleaf *p;

  fprintf(ofile, "\nvoid oui_init_options(int argc, char **argv) {\n");

  /* Vars */
  for (p = global_defs.packages.first; p != NULL; p = p->next)
	dump_llos( &p->pkg->vars, "" );

  if (switch_needed) output_switch();

  /* Inits */
  for (p = global_defs.packages.first; p != NULL; p = p->next)
	dump_llos( &p->pkg->inits, "" );

  fprintf(ofile, "}\n");
}

static void one_include(ll_of_str *ll, const char *s) {
  struct llosleaf *lll;
  
  for (lll = ll->first; lll != NULL; lll = lll->next)
	if (strcmp(lll->text, s) == 0) return;
  llos_enq(ll, s);
}

void output_includes(void) {
  llpkgleaf *p;
  ll_of_str prtd;
  char *s;

  prtd.first = prtd.last = NULL;
  one_include(&prtd, "\"oui.h\"");
  for (p = global_defs.packages.first; p != NULL; p = p->next) {
	while (s = llos_deq(&p->pkg->c_inc)) {
	  one_include(&prtd, s);
	  free_memory(s);
	}
  }
  dump_llos( &prtd, "#include " );
}

static int compar(const void *a, const void *b) {
  return(stricmp(*((const char **)a), *((const char **)b)));
}

static void output_sorted(void) {
  char **sorter;
  struct llosleaf *lf;
  int n_strs, i;
  
  if (sort_output) {
	n_strs = 0;
	for (lf = global_defs.sorted.first; lf != NULL; lf = lf->next)
	  n_strs++;
	if (n_strs > 0) {
	  sorter = new_memory(n_strs * sizeof(char *));
	  for (i = 0; i < n_strs; i++)
		sorter[i] = llos_deq(&global_defs.sorted);
	  qsort(sorter, n_strs, sizeof(char *), compar);
	  for (i = 0; i < n_strs; i++) {
		fprintf(ofile, "%s\n", sorter[i]);
		free_memory(sorter[i]);
	  }
	  free_memory(sorter);
	}
  } else dump_llos( &global_defs.sorted, "" );
}

void output_usage(void) {
  llpkgleaf *p;

  fprintf( ofile, "\n#ifdef __USAGE\n");
  
  /* Output the synopsis */
  if ( global_defs.synopsis == 0 )
	fprintf( ofile, "%%C\t[options]\n");
  else
	fprintf( ofile, "%s\n", global_defs.synopsis );

  /* Output the sorted options */
  output_sorted();
  
  /* and output the unsorted help */
  for (p = global_defs.packages.first; p != NULL; p = p->next)
	dump_llos( &p->pkg->unsort, "" );

  fprintf( ofile, "#endif\n");
}
