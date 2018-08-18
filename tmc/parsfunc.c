/* parsfunc.c
 * Revision 1.1  2008/07/03 15:11:07  ntallen
 * Copied from QNX4 version V1R9
 *
 * Revision 1.8  1999/08/30 17:15:14  nort
 * Changes relating to calibrations
 *
 * Revision 1.7  1994/08/02  16:00:58  nort
 * Added nortlib.h for new_memory().
 * Added dummy return codes for nr_validator and nr_declarator to stop warnings
 *
 * Revision 1.6  1993/09/27  19:35:43  nort
 * Changes for common compiler functions, cleanup of rlog, pragmas.
 *
 * Revision 1.5  1993/05/21  19:45:31  nort
 * Added State Variable Support
 */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "nortlib.h"
#include "rational.h"
#include "tmcstr.h"
#include "tmc.h"

struct statpc *program;
struct sttmnt initprog = {NULL, NULL};
struct statpc *newstpc(unsigned int type) {
  struct statpc *spc;

  spc = new_memory(sizeof(struct statpc));
  spc->type = type;
  spc->next = NULL;
  return(spc);
}  
struct statpc *newstpctext(char *text) {
  struct statpc *spc;
  
  if (text != NULL) {
    spc = newstpc(STATPC_TEXT);
    spc->u.text = text;
    return(spc);
  } else return(NULL);
}
struct statpc *common_stat(struct statpc *sp) {
  struct statpc *spc;
  
  spc = newstpc(STATPC_COMMON);
  spc->u.cmn = sp;
  return(spc);
}
void catstatpc(struct sttmnt *s, struct statpc *spc) {
  if (spc != NULL) {
    if (s->last == NULL) s->first = s->last = spc;
    else s->last = (s->last->next = spc);
  }
}
/* catstattext concatenates text onto the specified statement */
void catstattext(struct sttmnt *s, char *text) {
  catstatpc(s, newstpctext(text));
}
void initstat(struct sttmnt *s, struct statpc *spc) {
  s->first = s->last = spc;
}
/* catstat concatenates two statements */
void catstat(struct sttmnt *s1, struct sttmnt *s2) {
  if (s1->last == NULL) {
    assert(s1->first == NULL);
    *s1 = *s2;
  } else if (s2->last != NULL) {
    assert(s2->first != NULL);
    s1->last->next = s2->first;
    s1->last = s2->last;
  }
}
struct deplst *newdeplst(char *name, int once) {
  struct deplst *dl;
  
  dl = new_memory(sizeof(struct deplst));
  dl->next = NULL;
  dl->ref = find_name(name, 0);
  dl->flag = once;
  if (dl->ref == NULL) compile_error(2, "Undefined TM variable: %s", name);
  return(dl);
}
struct statpc *newdepend(void) {
  struct statpc *d;
  
  d = newstpc(STATPC_DEPEND);
  d->u.dep.vd = NULL;
  d->u.dep.sd = NULL;
  initstat(&d->u.dep.stat, NULL);
  initstat(&d->u.dep.else_stat, NULL);
  d->u.dep.rate.num = 0;
  d->u.dep.rate.den = 0;
  return(d);
}

struct nmlst *current_scope = NULL, *global_scope = NULL;

struct nmlst *newnamelist(struct nmlst *prev, struct nm *names) {
  struct nmlst *nl;
  
  nl = new_memory(sizeof(struct nmlst));
  nl->prev = prev;
  nl->names = names;
  return(nl);
}

void new_scope(void) {
  current_scope = newnamelist(current_scope, NULL);
  if (global_scope == NULL) global_scope = current_scope;
}

void old_scope(void) {
  assert(current_scope != NULL);
  assert(current_scope->prev != NULL);
  current_scope = current_scope->prev;
}

/* returns NULL if name isn't found. used by yylex */
struct nm *find_name(char *name, int declaring) {
  struct nmlst *nl;
  struct nm *nment;

  assert(current_scope != NULL);
  for (nl = current_scope; nl != NULL; nl = declaring ? NULL : nl->prev)
    for (nment = nl->names; nment != NULL; nment = nment->next)
      if (stricmp(name, nment->name) == 0) return(nment);
  return(NULL);
}

/* find ref always creates a ref if one isn't already there.
   declaring controls how far we look for a conflict. If declaring,
   we only look in the current scope.
*/
struct nm *find_ref(char *name, int declaring) {
  struct nm *nment;
  
  nment = find_name(name, declaring);
  if (nment == NULL) {
    nment = new_memory(sizeof(struct nm));
    nment->name = strdup(name);
    nment->type = NMTYPE_UNDEFINED;
    nment->next = current_scope->names;
    current_scope->names = nment;
  } else if (declaring && nment->type != NMTYPE_UNDEFINED)
    compile_error(2, "Redefinition of symbol %s", name);
  return(nment);
}

struct statpc *find_ref_spc(char *name, int declaring) {
  struct statpc *spc;
  
  spc = newstpc(STATPC_REF);
  spc->u.nameref = find_ref(name, declaring);
  return(spc);
}

struct validator *new_validator(void) {
  struct validator *nv;
  
  nv = new_memory(sizeof(struct validator));
  nv->flag = 0;
  nv->val = NULL;
  initstat(&nv->valstat, NULL);
  return(nv);
}

void link_declarator(struct declrtor *decl, unsigned int type) {
  struct dtm *dd;
  struct tmdtm *tmd;
  struct tmtype *tmt;

  decl->nameref->type = type;
  switch (type) {
    case NMTYPE_DATUM:
      dd = new_memory(sizeof(struct dtm));
      decl->nameref->u.ddecl = dd;
      dd->name = decl->nameref->name;
      dd->decl = decl;
      dd->vldtr = new_validator();
      break;
    case NMTYPE_TMDATUM:
      tmd = new_memory(sizeof(struct tmdtm));
      tmd->name = decl->nameref->name;
      tmd->decl = decl;
      tmd->tmdef = new_tmalloc(decl->nameref);
      initstat(&tmd->collect, NULL);
      decl->nameref->type = NMTYPE_TMDATUM;
      decl->nameref->u.tmdecl = tmd;
      break;
    case NMTYPE_TMTYPE:
      tmt = new_memory(sizeof(struct tmtype));
      clr_tmtype(tmt);
      tmt->decl = decl;
      decl->nameref->u.tmtdecl = tmt; /* filled in after parse */
      break;
    default:
      compile_error(4, "Unknown decl_type %d in tmc.y", type);
  }
}

void clr_tmtype(struct tmtype *tmt) {
  struct caldef *cdf;

  tmt->decl = NULL;
  tmt->dummy = NULL;
  initstat(&tmt->collect, NULL);
  tmt->convert = NULL;
  cdf = &tmt->caldefs;
  cdf->convclass = CV_UK;
  cdf->cal = NULL;
  cdf->cvt = cdf->icvt = cdf->tcvt = NULL;
  cdf->yscale = 0;
  tmt->txtfmt = NULL;
}

/* nr_declarator returns the declarator associated with the given
   nameref. */
struct declrtor *nr_declarator(struct nm *nameref) {
  if (!name_test(nameref, NMTEST_DECLARATOR))
    compile_error(4, "Undefined name %s in nr_declarator", nameref->name);
  switch (nameref->type) {
    case NMTYPE_DATUM:
      return(nameref->u.ddecl->decl);
    case NMTYPE_TMDATUM:
      return(nameref->u.tmdecl->decl);
    case NMTYPE_TMTYPE:
      return(nameref->u.tmtdecl->decl);
    default:
      compile_error(4, "Unsupported type %d in nr_declarator", nameref->type);
  }
  return 0;
}

/* nameref must be validatable, or nr_validator will die with an
   error message
 */
struct validator *nr_validator(struct nm *nameref) {
  assert(name_test(nameref, NMTEST_VALID));
  switch (nameref->type) {
    case NMTYPE_DATUM:
      return(nameref->u.ddecl->vldtr);
    case NMTYPE_STATE:
      return(nameref->u.stdecl->vldtr);
    default:
      compile_error(4, "Unexpected type %d in nr_validator", nameref->type);
  }
  return 0;
}

struct tmalloc *nr_tmalloc(struct nm * nameref) {
  assert(name_test(nameref, NMTEST_TMALLOC));
  if (nameref->type == NMTYPE_TMDATUM)
    return(nameref->u.tmdecl->tmdef);
  else return(nameref->u.grpd->tmdef);
}

/* name_test provides tests for groups of nameref types. The test
   codes are defined in tmc.h with the prefix NMTEST_.
            NMTYPE_UNDEFINED 0
            NMTYPE_TMTYPE 1
            NMTYPE_DATUM 2
            NMTYPE_TMDATUM 3
            NMTYPE_GROUP 4
            NMTYPE_DUMMY 5
            NMTYPE_STATE 6
*/
#define MAX_NTESTS 8
static int ntmask[MAX_NTESTS] = {
  0xC,  /* NMTEST_DATA = NMTYPE_DATUM | NMTYPE_TMDATUM */
  0x2,  /* NMTEST_TYPE = NMTYPE_TMTYPE */
  0x8,  /* NMTEST_TMDATUM = NMTYPE_TMDATUM */
  0x10, /* NMTEST_GROUP = NMTYPE_GROUP */
  0x44, /* NMTEST_VALID = NMTYPE_DATUM | NMTYPE_STATE (validatable) */
  0xE,  /* NMTEST_DECLARATOR = _TMTYPE, _DATUM, _TMDATUM */
  0x18, /* NMTEST_TMALLOC = _GROUP | _TMDATUM */
  0x4   /* NMTEST_INVALID = TMTYPE_DATUM (invalidatable) */
};

int name_test(struct nm *nameref, unsigned int testtype) {
  assert(nameref != NULL);
  assert(nameref->name != NULL);
  assert(testtype < MAX_NTESTS);
  switch (nameref->type) {
    case NMTYPE_UNDEFINED:
      break;
    case NMTYPE_TMTYPE:
      assert(nameref->u.tmtdecl != NULL);
      assert(nameref->u.tmtdecl->decl != NULL);
      break;
    case NMTYPE_DATUM:
      assert(nameref->u.ddecl != NULL);
      assert(nameref->u.ddecl->decl != NULL);
      assert(nameref->u.ddecl->name != NULL);
      assert(nameref->u.ddecl->vldtr != NULL);
      break;
    case NMTYPE_TMDATUM:
      assert(nameref->u.tmdecl != NULL);
      assert(nameref->u.tmdecl->name != NULL);
      assert(nameref->u.tmdecl->decl != NULL);
      assert(nameref->u.tmdecl->tmdef != NULL);
      break;
    case NMTYPE_GROUP:
      assert(nameref->u.grpd != NULL);
      assert(nameref->u.grpd->tmdef != NULL);
      break;
    case NMTYPE_STATE:
      assert(nameref->u.stdecl != NULL);
      assert(nameref->u.stdecl->vldtr != NULL);
      assert(nameref->u.stdecl->funcname != NULL);
      break;
  }
  return((1 << nameref->type) & ntmask[testtype]);
}

struct tmalloc *new_tmalloc(struct nm *nr) {
  struct tmalloc *tma;
  
  assert(nr != NULL);
  tma = new_memory(sizeof(struct tmalloc));
  tma->next = NULL;
  tma->flags = Collecting ? 0 : TMDF_HOMEROW;
  tma->nameref = nr;
  tma->size = 0;
  tma->rate = zero;
  tma->sltcw = NULL;
  return(tma);
}
