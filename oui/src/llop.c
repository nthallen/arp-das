/* llop.c contains linked list of package functions
 * $Log$
 */
#include <string.h>
#include "nortlib.h"
#include "compiler.h"
#include "ouidefs.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

void llopkg_enq(ll_of_pkg *llp, const package *pkg) {
  llpkgleaf *lf;

  if (llp != 0) {
	{ int resp = set_response(3);
	  lf = new_memory( sizeof( llpkgleaf ) );
	  set_response(resp);
	}
	lf->pkg = pkg;
	lf->next = 0;
	if (llp->last == 0)
	  llp->first = llp->last = lf;
	else {
	  llp->last->next = lf;
	  llp->last = lf;
	}
  }
}

package *find_package(const char *pkgname) {
  llpkgleaf *lf;
  package *pkg;
  static int n_packages = 0;

  if (pkgname == 0 || pkgname[0] == '\0')
	compile_error(4, "Back pkgname in find_package");
  for (lf = global_defs.packages.first; lf != 0; lf = lf->next) {
	if (stricmp(pkgname, lf->pkg->name) == 0)
	  return lf->pkg;
  }
  pkg = new_memory(sizeof(package));
  pkg->package_id = n_packages++;
  pkg->name = nl_strdup(pkgname);
  pkg->opt_string = 0;
  pkg->c_inc.first = pkg->c_inc.last = 0;
  pkg->defs.first = pkg->defs.last = 0;
  pkg->vars.first = pkg->vars.last = 0;
  pkg->switches.first = pkg->switches.last = 0;
  pkg->arg.first = pkg->arg.last = 0;
  pkg->inits.first = pkg->inits.last = 0;
  pkg->unsort.first = pkg->unsort.last = 0;
  pkg->preceed.first = pkg->preceed.last = 0;
  pkg->follow = 0;
  pkg->flags = 0;
  llopkg_enq( &global_defs.packages, pkg );
  return(pkg);
}
