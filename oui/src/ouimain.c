#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "nortlib.h"
#include "compiler.h"
#include "ouidefs.h"
#include "oui.h"

int switch_needed = 0, arg_needed = 0;
int sort_output = 1;
glbldef global_defs;

/* This stuff is handy when the heap gets corrupted */
/* #define CHECK_HEAP heap_check(); */
#ifdef CHECK_HEAP
  #include <malloc.h>
  void heap_check( void ) {
	struct _heapinfo h_info;
	int heap_status;
  
	h_info._pentry = NULL;
	for (;;) {
	  heap_status = _heapwalk( &h_info );
	  if ( heap_status != _HEAPOK ) break;
	}
	switch ( heap_status ) {
	  case _HEAPEND:
	  case _HEAPEMPTY:
		break;
	  default:
		nl_error( 1, "Internal: Heap is corrupted" );
		break;
	}
  }
#else
  #define CHECK_HEAP
#endif

int main(int argc, char **argv) {
  unsigned int errlevel;

  oui_init_options(argc, argv);
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

/* check_new_opts() reports on any options incompatabilities, but
   it doesn't actually terminate, so additional incompatabilities
   can be reported.
*/
static void check_new_opts(const char *opts, char *pkgname) {
  char *buf, *op;
  const char *o;
  llpkgleaf *p;

  buf = new_memory( strlen(opts) + 1 );
  for (o = opts; isspace(*o); o++); /* skip whitespace */

  /* collect the option letters from opts into buf */
  for (op = buf; *o != '\0'; o++) {
	if (*o != ':') *op++ = *o;
  }
  *op = '\0';

  /* now check these against all the other packages */
  for (p = global_defs.packages.first; p != NULL; p = p->next) {
	op = p->pkg->opt_string;
	while (op != NULL) {
	  op = strpbrk(op, buf);
	  if (op != NULL) {
		nl_error(2, "Package %s option -%c conflicts with package %s",
		  pkgname, *op, p->pkg->name);
		op++;
	  }
	}
  }

  free_memory(buf);
}

void oui_opts(char *str) {
  if (global_defs.crnt_pkg != 0) {
	if (global_defs.crnt_pkg->opt_string == 0) {
	  /* want to check option against existing options */
	  check_new_opts(str, global_defs.crnt_pkg->name);
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
