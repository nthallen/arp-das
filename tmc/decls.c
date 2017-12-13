/* decls.c Functionality removed from tmc.y for better understanding
 * Revision 1.2  2008/07/03 18:18:48  ntallen
 * To compile under QNX6 with minor blind adaptations to changes between
 * dbr.h and tm.h
 *
 * Revision 1.1  2008/07/03 15:11:07  ntallen
 * Copied from QNX4 version V1R9
 *
 * Revision 1.2  1993/09/27 19:41:29  nort
 * pragmas, typo.
 *
 * Revision 1.1  1993/07/09  19:41:14  nort
 * Initial revision
 *
 */
#include <stdio.h>
#include <assert.h>
#include "rational.h"
#include "tmcstr.h"
#include "calibr.h"
#include "yytype.h"
#include "tmc.h"

void int_type(struct typparts *tp, char *text, unsigned int type) {
  initstat(&tp->stat, newstpctext(text));
  tp->type = type;
}

/* Handles the start of a struct or union. Returns NMTYPE_DATUM
   always for defining decl_type.
*/
unsigned int start_st_un(struct st_un *su, char *text,
            unsigned int type, unsigned int dclt) {
  new_scope();
  initstat(&su->stat, newstpctext(text));
  su->type = type;
  su->decl_type = dclt;
  if (type == INTTYPE_STRUCT)
    catstattext(&su->stat, " __attribute__((__packed__))");
  return(NMTYPE_DATUM);
}

/* Handles the total definition of a struct or union.
   Returns the previous value of decl_type.
*/
unsigned int end_st_un(struct typparts *tp, struct st_un *su,
        char *pre, struct sttmnt *decls, char *post) {
  struct statpc *spc;

  tp->stat = su->stat;
  catstattext(&tp->stat, pre);
  tp->tm_type = NULL;
  tp->type = su->type;
  tp->size = 0;
  for (spc = decls->first; spc != NULL; spc = spc->next) {
    if (spc->type == STATPC_DECLS) {
      assert(spc->u.decls != NULL);
      if (su->type == INTTYPE_STRUCT)
        tp->size += spc->u.decls->size;
      else if (spc->u.decls->size > tp->size)
        tp->size = spc->u.decls->size;
    } else assert(spc->type == STATPC_TEXT);
  }
  catstat(&tp->stat, decls);
  catstattext(&tp->stat, post);
  old_scope();
  return(su->decl_type);
}

/* Fills in a typparts structure */
void set_typpts(struct typparts *tp, unsigned int type, unsigned int size,
      char *text, struct tmtype *tmt) {
  tp->type = type;
  tp->size = size;
  if (text != NULL) initstat(&tp->stat, newstpctext(text));
  else initstat(&tp->stat, NULL);
  tp->tm_type = tmt;
}
