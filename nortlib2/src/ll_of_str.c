/* llos defines a linked list of strings type
 * $Log$
 */
#include "nortlib.h"
#include "ll_of_str.h"

/*
Functionality:
  void llos_enq(ll_of_str *ll, const char *str);
	duplicates str and places it on the list.
	Dies if memory allocation fails.
	Accepts NULL in either argument. ll==NULL does nothing.
	str == NULL gets mapped into empty string "".
  char *llos_deq(ll_of_str *ll);
	produces the duplicated string at the head of the list
	and deletes it from the list. The string should be
	freed (free_memory()) after it is used, since it was
	dynamically allocated when enqueued.

Possible additional functionality: Iteration.
    init
	next (yielding first after init)
	insert_before
	insert_after
	delete
*/

void llos_enq(ll_of_str *ll, const char *str) {
  char *s;
  struct llosleaf *lf;

  if (ll != 0) {
	{ int resp = set_response(3);
	  lf = new_memory( sizeof( struct llosleaf ) );
	  if (str == 0) s = nl_strdup("");
	  else s = nl_strdup(str);
	  set_response(resp);
	}
	lf->text = s;
	lf->next = 0;
	if (ll->last == 0)
	  ll->first = ll->last = lf;
	else {
	  ll->last->next = lf;
	  ll->last = lf;
	}
  }
}

char *llos_deq(ll_of_str *ll) {
  char *s;
  struct llosleaf *lf;

  if (ll == 0 || ll->first == 0) return 0;
  lf = ll->first;
  s = lf->text;
  ll->first = lf->next;
  if (ll->first == 0)
	ll->last = 0;
  free_memory(lf);
  return s;
}
