/* sortpkg.c contains sort_packages()
 * $Log$
 */
#include <assert.h>
#include "nortlib.h"
#include "ouidefs.h"

static int report_cycle(package *pkg) {
  llpkgleaf *p;

  if ((pkg->flags & PKGFLG_VISITED) == 0) {
	pkg->flags |= PKGFLG_VISITED;
	for (p = pkg->preceed.first; p != 0; p = p->next)
	  if (report_cycle(p->pkg))
		break;
	if (p == 0)
	  pkg->flags &= ~PKGFLG_VISITED;
  }
  if (pkg->flags & PKGFLG_VISITED) {
	nl_error(2, "Package in cycle: %s", pkg->name);
	return 1;
  }
  return 0;
}

void sort_packages(void) {
  llpkgleaf **pp, *p, *unsrt;
  package *pkg;

  unsrt = global_defs.packages.first;
  global_defs.packages.first = global_defs.packages.last = 0;  
  while (unsrt != 0) {
	/* find the first package with no preceeding packages */
	pp = &unsrt;
	p = *pp;
	while (p != 0 && p->pkg->follow != 0) {
	  pp = &(p->next);
	  p = p->next;
	}
	if (p == 0) {
	  report_cycle(unsrt->pkg);
	  return;
	}
	
	/* Now add it to the new list if defined */
	pkg = p->pkg;
	if (pkg->flags & PKGFLG_DEFINED)
	  llopkg_enq(&global_defs.packages, pkg);

	/* and delete it from the old list */
	*pp = p->next;
	free_memory(p);
	
	/* Then decrement follow for each of the preceed packages */
	for (p = pkg->preceed.first; p != 0; p = pkg->preceed.first) {
	  assert(p->pkg != 0 && p->pkg->follow > 0);
	  p->pkg->follow--;
	  pkg->preceed.first = p->next;
	  free_memory(p);
	}
  }
}
