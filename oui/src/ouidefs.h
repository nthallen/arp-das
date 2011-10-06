/* oui.h Data definitions for oui
 * $Log$
 * Revision 1.2  1994/09/16 15:09:36  nort
 * Added sort_output
 *
 * Revision 1.1  1994/09/15  19:45:31  nort
 * Initial revision
 *
 */
#ifndef _OUI_H
#define _OUI_H

#include "config.h"

#if HAVE_STRCASECMP
  #if ! HAVE_STRICMP
    #define stricmp strcasecmp
  #endif
#endif

#ifndef _LLOFSTR_INCLUDED
  #include "ll_of_str.h"
#endif

typedef struct llpkglf {
  struct llpkglf *next;
  struct pkg *pkg;
} llpkgleaf;

typedef struct llpkg {
  llpkgleaf *first;
  llpkgleaf *last;
} ll_of_pkg;

typedef struct pkg {
  int package_id; /* unique for comparison */
  char *name;
  char *opt_string;
  ll_of_str c_inc;
  ll_of_str defs;
  ll_of_str vars;
  ll_of_str inits;
  ll_of_str unsort;
  ll_of_str switches;
  ll_of_str arg;
  ll_of_pkg preceed; /* list of packages this one preceeds */
  int follow; /* count of packages this one follows */
  int flags; /* bit 0 is defined */
  /* (List of conflicting packages) */
} package;
#define PKGFLG_DEFINED 1
#define PKGFLG_VISITED 2

void llopkg_enq(ll_of_pkg *llp, const package *pkg);
package *find_package(const char *pkgname);

typedef struct {
  ll_of_pkg packages;
  package *crnt_pkg;
  char *synopsis;
  ll_of_str sorted; /* sorted usage */
} glbldef;

extern glbldef global_defs;

typedef union {
  char *strval;
  package *pkgval;
} yystype;
#define YYSTYPE yystype

extern YYSTYPE yyval, yylval;

void oui_opts(char *str);
void oui_inits(char *str);
void oui_c_include(char *str);
void oui_defs(char *str);
void oui_vars(char *str);
void oui_unsort(char *str);
void oui_synopsis(char *str);
void oui_sort(char *str);
void oui_prepkg(package *pkg);
void oui_folpkg(package *pkg);
void oui_include(char *str);
void oui_defpkg(package *pkg);
void oui_switch(char *str);
void oui_arg(char *str);
extern int switch_needed, arg_needed;
void sort_packages(void);
void output_comments(void);
void output_opt_string(void);
void output_defs(void);
void output_inits(void);
void output_usage(void);
void output_includes(void);
extern int sort_output;

#endif
