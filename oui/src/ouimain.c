#include <stdlib.h>
#include "nortlib.h"
#include "compiler.h"
#include "ouidefs.h"

#ifdef __USAGE
%C	[-qwvk] [-o filename] file [file ...]
	-q Print usage message
	-w Treat warnings as errors
	-v Increasing level of verbosity
	-k Keep incomplete output file on error
	-o Specify Output Filename
#endif

char *opt_string = OPT_COMPILER_INIT;
void (*nl_error)(int level, char *format, ...) = compile_error;

int switch_needed = 0, arg_needed = 0;
glbldef global_defs;

void main(int argc, char **argv) {
  unsigned int errlevel;

  compile_init_options(argc, argv, ".c");
  errlevel = yyparse();
  if (error_level < errlevel) error_level = errlevel;
  if (error_level == 0) {
	sort_packages();
	output_comments();
	output_opt_string();
	output_includes();
	output_defs();
	output_inits();
	output_usage();
  }
  if (error_level) fprintf(stderr, "Error level %d\n", error_level);
  exit(error_level);
}

void oui_opts(char *str) {
  if (global_defs.crnt_pkg != 0) {
	if (global_defs.crnt_pkg->opt_string == 0) {
	  /* want to check option against existing options */
	  global_defs.crnt_pkg->opt_string = str;
	} else compile_error(2,
	  "Redefinition of option string for package %s",
	  global_defs.crnt_pkg->name);
  }
}

void oui_inits(char *str) {
  if (global_defs.crnt_pkg != 0)
	llos_enq( &global_defs.crnt_pkg->inits, str );
  free_memory(str);
}

void oui_defs(char *str) {
  if (global_defs.crnt_pkg != 0)
	llos_enq( &global_defs.crnt_pkg->defs, str );
  free_memory(str);
}

void oui_unsort(char *str) {
  if (global_defs.crnt_pkg != 0)
	llos_enq( &global_defs.crnt_pkg->unsort, str );
  free_memory(str);
}

void oui_synopsis(char *str) {
  if (global_defs.synopsis == 0)
	global_defs.synopsis = str;
  else compile_error(2, "Redefinition of synopsis");
}

void oui_sort(char *str) {
  if (global_defs.crnt_pkg != 0)
	llos_enq( &global_defs.sorted, str );
  free_memory(str);
}

void oui_prepkg(package *pkg) {
  if (global_defs.crnt_pkg != NULL) {
	pkg->follow++;
	llopkg_enq( & global_defs.crnt_pkg->preceed, pkg);
  }
}

void oui_folpkg(package *pkg) {
  if (global_defs.crnt_pkg != NULL) {
	global_defs.crnt_pkg->follow++;
	llopkg_enq( & pkg->preceed, global_defs.crnt_pkg);
  }
}

void oui_include(char *str) {
  if (global_defs.crnt_pkg != NULL)
	llos_enq(&input_files, str);
  free_memory(str);
}

void oui_defpkg(package *pkg) {
  if (pkg->flags & PKGFLG_DEFINED) global_defs.crnt_pkg = 0;
  else {
	pkg->flags |= PKGFLG_DEFINED;
	global_defs.crnt_pkg = pkg;
  }
}

void oui_vars(char *str) {
  if (global_defs.crnt_pkg != 0)
	llos_enq( &global_defs.crnt_pkg->vars, str );
  free_memory(str);
}

void oui_c_include(char *str) {
  if (global_defs.crnt_pkg != 0)
	llos_enq( &global_defs.crnt_pkg->c_inc, str );
  free_memory(str);
}

static void need_switch(void) {
  if (!switch_needed) {
	llos_enq( &global_defs.crnt_pkg->c_inc, "<unistd.h>");
	llos_enq( &global_defs.crnt_pkg->c_inc, "\"nortlib.h\"");
	switch_needed = 1;
  }
}

void oui_switch(char *str) {
  if (global_defs.crnt_pkg != 0) {
	llos_enq( &global_defs.crnt_pkg->switches, str );
	need_switch();
  }
  free_memory(str);
}

void oui_arg(char *str) {
  if (global_defs.crnt_pkg != 0) {
	llos_enq( &global_defs.crnt_pkg->arg, str );
	need_switch();
	arg_needed = 1;
  }
  free_memory(str);
}
