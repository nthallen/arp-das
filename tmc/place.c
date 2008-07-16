/* place.c
 * $Log$
 * Revision 1.3  2008/07/16 18:55:14  ntallen
 * Changes to support TM_Data_Type 3
 *
 * Revision 1.2  2008/07/03 18:18:48  ntallen
 * To compile under QNX6 with minor blind adaptations to changes between
 * dbr.h and tm.h
 *
 * Revision 1.1  2008/07/03 15:11:07  ntallen
 * Copied from QNX4 version V1R9
 *
 * Revision 1.10  1995/10/18 02:03:08  nort
 * *** empty log message ***
 *
 * Revision 1.9  1993/09/27  19:37:23  nort
 * Change relating to states.
 *
 * Revision 1.8  1993/07/13  19:44:05  nort
 * Place "validate <state>" on initprog
 *
 * Revision 1.7  1993/07/09  19:40:59  nort
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "nortlib.h"
#include "rational.h"
#include "tmcstr.h"
#include "tmc.h"

static char rcsid[] =
      "$Id$";

/*
How do we depend {
  A statement which validates a datum cannot be dependent on that
  datum, or the datum will never be validated. {
    Unless explicitly depending on datum, validate x should
    REMOVE x from the dependency list.
  }
  summary {
    Collect actions do not depend.
    extraction statements depend on variables with possible "once" qual.
    dependence on a variable can be broken down into zero or more of {
      positional dependence
      simple slot dependence
      explicit dependence (i.e. response to [in]validation commands)
    }
    +Dependence on a non-TM variable with no invalidation can be ignored.
    Simple slot dependence can be ignored if collecting, since no slots
    will be lost. When not collecting, slot dependence can be relaxed to
    Minor Frame dependence, since the integrity of the minor frame is
    guaranteed at a lower level.
    sub-statement dependence allows the enclosing statement to be
    independent of references within the substatement except that
    positional dependence is still an issue. sub-statement positional
    information should not affect the RATE of the enclosing statement,
    but it should affect the position to assure validity.
  }
  implementation {
    Ultimately, dependency is recorded as two lists: {
      {non-TM variable, 'once' flag}
      {position, slots}
    }
    When evaluating a dependency, must know whether position is
    to be added loosely or strictly. {
      Need two slot lists and a variable list
      in substat, add all slots to 'local' list and apply loosely
      to the global list.
      at stat level, add all slots to global list and apply strictly
      to global list.
      Hence, if there is a local list, apply slots there and loosely
      to the global, else a
      in substat, first prune existing deplst for redundancy, dependence
      on unnecessary variables.
    }
    'loose' positional dependencies have affected the position but
    do not appear on the list of slots.
    If no positional info, then must use 'active' validation,
    separate function.
    Assign bits for each element
    determine type to use for storage {
      use only unsigned char and unsigned int.
      Use more than one if necessary.
    }
    Allocate names for the storage elements
    Add validation records to slots and variables {
      storage element name
      bit to set or reset, depending on whether validating or
      invalidating
      Function name to call for active validation {
        validate x; would translate to
          val123 &= ~0x80;
          funcname();
        The function would have to check for validity, since
        it might require more than one value:
        void funcname(void) {
          if (val123 == 0 && val124 == 0) {
            val123 |= 0x11;
            block
          }
        }
      }
    }
  }
  TM Dependence {
    Dependence on a TM variable is {
      positional dependence on its last slot {
        strict: affects rate
        loose: does not affect rate, only fine position
      }
      and slot dependence on each slot {
        After the slot position of the action has been defined,
        dependence on compatable slots can be removed.
        If collecting, remove slot dependence on all
        slots with the same or smaller period than the action.
        If 'First' slot appears in a different 
        then each slot should validate itself
      }
    }
    sub-statement dependence {
      Evaluate normally
      Apply positional dependencies to enclosing statement loosely
    }
  }
  Slot Dependence {
    Simple slot dependence is brought about by implicit dependence
    on a TM variable which is located in whole or in part in a slot
    which is incompatable with that of the action. This is not a
    "once" dependancy and is only invalidated when the row or mfc
    containing the variable is skipped for some reason. During
    collection, there are no skipped rows, hence no invalidation,
    hence the simple slot dependence can be ignored. (This saves
    not only the checking before the action, but also the validation
    whenever the row appears.)
    
    Sub-statement dependence on a TM variable produces a more complex
    slot dependency. 
    NOT! Not inherently "once" #### Must ask: {
      is it ever invalid? {
        split between multiple slots
      }
      is the range of that invalidity critical?
      In general, do not support this case!
    }
  }
  On derived data (non-TM) {
    Derived data are assumed always valid unless explicitly invalidated
    by the user. Hence, if no "invalidate x" statements are present, x
    is assumed always valid and any dependency on x is ignored.
    "validate x;" or "invalidate x;" at the top level can define the
    initial conditions. Only the latter triggers validation however.
    "invalidate x" at any level triggers validation on x both implicit
    and explicit. Dependency on derived data cannot be positional,
    and hence must always be implemented explicitly via validation
    records.
    Even with validation triggered, a statement cannot be dependent
    on a derived datum if it validates that datum or the datum
    will never get validated. {
      I could split hairs on this, but I think the simple solution
      is to have a "validate x;" statement remove dependence on x
      unless that dependence is explicit (depending on (x){}).
      I can't think of a case where this would be desired, but
      then again, I'm not writing programs yet.
    }
  }
}
*/

#define IS_DEPEND(x) ((x) == STATPC_DEPEND)
static void chk_deps(struct statpc *rule, struct statpc *dep);

static unsigned short get_rate_period(rational *rate) {
  rational v;
  
  if (rate->num == 0) return(0);
  v = Rsynch;
  v.num *= SynchPer;
  rdivide(&v, rate, &v);
  if (v.den != 1)
    compile_error(3, "Rate %d/%d Hz does not map to integral period",
      rate->num, rate->den);
  return(v.num);
}

static void chk_slot(struct nm *ref, struct slt *slot) {
  struct slt *refslot;
  struct tmalloc *tma;

  if (name_test(ref, NMTEST_TMDATUM)) {
    tma = ref->u.tmdecl->tmdef;
    if (tma->flags & TMDF_HOMEROW) {
      refslot = tma->sltcw->slot;
      if (slot == NULL
          || refslot->per < slot->per
          || refslot->row % slot->per != slot->row
          || tma->sltcw->dnext != NULL)
        tma->flags &= ~TMDF_HOMEROW;
    }
  }
}

/* chk_refs checks the rule for global references.
   Notes:
    STATPC_DEFRUL: dummies only appear in collection rules and
    always refer to the datum being collected. Hence the reference
    needn't be checked. [in]validation on dummies is illegal since
    dummies are TM variables. Dependence on a dummy is moot just
    as the reference is.
    STATPC_CONVERT: STATPC_REF must follow and is handled as any
    other _REF, but we take the opportunity to record the need
    for the conversion function with the tmtype.
*/
static void chk_refs(struct slt *slot, struct statpc *spc) {
  struct deplst *dl;

  for (; spc != NULL; spc = spc->next) {
    switch (spc->type) {
      case STATPC_CONVERT:
        assert(spc->u.cvt.ref != NULL && spc->u.cvt.ref->type == STATPC_REF);
        chk_refs(slot, spc->u.cvt.ref);
        /* specify_conv(spc); moved to calibr.c */
        break;
      case STATPC_REF:
        chk_slot(spc->u.nameref, slot);
        break;
      case STATPC_DEFRUL:
        chk_refs(slot, spc->u.defrule.stat.first);
        break;
      case STATPC_DEPEND:
        for (dl = spc->u.dep.vd; dl != NULL; dl = dl->next)
          chk_slot(dl->ref, slot);
        chk_refs(slot, spc->u.dep.stat.first);
        chk_refs(slot, spc->u.dep.else_stat.first);
        break;
      case STATPC_COMMON:
        chk_refs(slot, spc->u.cmn);
        break;
      case STATPC_VALID:
      case STATPC_INVALID:
      case STATPC_TEXT:
      case STATPC_DECLS:
      case STATPC_ADDR:
      case STATPC_VALCHK:
        /* Nothing to do! */
        break;
      case STATPC_EXTRACT:
      case STATPC_TLDECLS:
      default:
        compile_error(4, "Unexpected STATPC type %d in chk_refs", spc->type);
        break;
    }
  }
}

/* place_col positions collection statements with the appropriate
   slots.
*/
void place_col(void) {
  struct nm *name;
  struct tmdtm *datum;
  struct sttmnt colrule;
  struct statpc *spc;

  for (name = global_scope->names; name != NULL; name = name->next) {
    /* Select the correct datum */
    if (name_test(name, NMTEST_TMDATUM) &&
        !(name->u.tmdecl->tmdef->flags & TMDF_GRPMEM))
      datum = name->u.tmdecl;
    else if (name->type == NMTYPE_GROUP)
      datum = name->u.grpd->grpmems->names->u.tmdecl;
    else continue;

    /* Locate a collection rule */
    assert(datum->decl != NULL);
    if (datum->collect.first != NULL) colrule = datum->collect;
    else if (datum->decl->tm_type != NULL &&
              datum->decl->tm_type->collect.first != NULL) {
      initstat(&colrule, newstpc(STATPC_DEFRUL));
      colrule.first->u.defrule.dummy = datum->decl->tm_type->dummy;
      colrule.first->u.defrule.nameref = name;
      colrule.first->u.defrule.stat = datum->decl->tm_type->collect;
    } else continue;

    /* call chk_refs to check all references within action. */
    assert(datum->tmdef->sltcw != NULL);
    chk_refs(datum->tmdef->sltcw->slot, colrule.first);
    spc = newstpc(STATPC_TEXT);
    spc->u.text = NULL;
    catstatpc(&datum->tmdef->sltcw->slot->colacts, spc);
    catstat(&datum->tmdef->sltcw->slot->colacts, &colrule);
  }
}

/* place_valid handles top-level in/validation statements */
void place_valid(void) {
  struct statpc *spc;
  struct nm *var;
  struct validator *vldtr;

  for (spc = program; spc != NULL; spc = spc->next) {
    switch (spc->type) {
      case STATPC_VALID:
      case STATPC_INVALID:
        var = spc->u.nameref;
        if (!name_test(var, NMTEST_VALID))
          compile_error(4, "%s is not validatable", var->name);
        vldtr = nr_validator(var);
        if (vldtr->flag & DCLF_TLSET)
          compile_error(2, "Attempt to redefine validity of %s", var->name);
        else {
          vldtr->flag |= DCLF_TLSET;
          if (spc->type == STATPC_INVALID) vldtr->flag &= ~DCLF_VALID;
          else assert(vldtr->flag & DCLF_VALID);
        }
        break;
      case STATPC_TLDECLS:
      case STATPC_EXTRACT:
        break;
      default:
        compile_error(4, "Unexpected STATPC type %d in place_valid", spc->type);
        break;
    }
  }
}

static void add_sdep(struct statpc *dep, struct slt *slot,
                      unsigned int flag) {
  struct sltdep *sd, *nsd;

  for (sd = dep->u.dep.sd; sd != NULL; sd = sd->next)
    if (sd->per == slot->per && sd->row == slot->row) break;
  if (sd != NULL) /* already listed */
    sd->flag |= flag;
  else {
    nsd = new_memory(sizeof(struct sltdep));
    nsd->per = slot->per; nsd->row = slot->row;
    nsd->flag = flag; nsd->next = dep->u.dep.sd;
    dep->u.dep.sd = nsd;
  }
}

/* Called twice by chk_deps to add slots to the sltdep list */
static void add_sdeps(struct statpc *dep, struct cw *cwl, unsigned int flag) {
  assert(cwl != NULL);
  if (cwl->dnext != NULL && (flag & DEPL_ONCE) == 0)
    flag |= DEPL_SPLIT;
  for ( ; cwl != NULL; cwl = cwl->dnext)
    add_sdep(dep, cwl->slot, flag);
}

/* evaluate rule elements for references, adding to dep->u.dep.vd
  A statement which validates a datum cannot be dependent on that
  datum, or the datum will never be validated. {
    Unless explicitly depending on datum, validate x should
    REMOVE x from the dependency list.
  }
*/
static void chk_ldeps(struct statpc *rule, struct statpc *dep) {
  struct nm *ref;
  struct deplst *vd, *nvd;

  for (; rule != NULL; rule = rule->next) {
    switch (rule->type) {
      case STATPC_TEXT:
      case STATPC_DECLS:
      case STATPC_INVALID:
        break;
      case STATPC_REF:
      case STATPC_VALID:
        ref = rule->u.nameref;
        switch (ref->type) {
          case NMTYPE_TMDATUM:
            add_sdeps(dep, ref->u.tmdecl->tmdef->sltcw, 0);
            break;
          case NMTYPE_DATUM:
            if (rule->type == STATPC_REF) {
              if (ref->u.ddecl->vldtr->flag & DCLF_VALIDATE) {
                for (vd = dep->u.dep.vd;
                     vd != NULL && vd->ref != ref;
                     vd = vd->next);
                if (vd == NULL) {
                  vd = new_memory(sizeof(struct deplst));
                  vd->next = dep->u.dep.vd;
                  vd->ref = ref;
                  vd->flag = 0;
                  dep->u.dep.vd = vd;
                }
              }
            } else { /* remove ref from dependencies */
              for (nvd = NULL;;) {
                vd = (nvd == NULL) ? dep->u.dep.vd : nvd->next;
                if (vd == NULL) break;
                if (vd->ref == ref) {
                  if (nvd == NULL) dep->u.dep.vd = vd->next;
                  else nvd->next = vd->next;
                  free_memory(vd);
                } else nvd = vd;
              }
            }
            break;
          default:
            break;
        }
        break;
      case STATPC_CONVERT:
        chk_ldeps(rule->u.cvt.ref, dep);
        break;
      case STATPC_DEPEND:
        chk_deps(rule->u.dep.stat.first, rule);
        chk_ldeps(rule->u.dep.else_stat.first, dep);
        break;
      case STATPC_COMMON:
        chk_ldeps(rule->u.cmn, dep);
        break;
      case STATPC_DEFRUL:
      case STATPC_TLDECLS:
      case STATPC_EXTRACT:
      default:
        compile_error(4, "Unexpected statpc type %d in chk_ldeps", rule->type);
        break;
    }
  }
}

/* chk_deps checks a statement for dependencies, adding info to the
   slot list and the variable list. Does not calculate positional
   dependency at all. Recurses for nested statements
   STATPC_DEPEND. Any slotted variable dependencies are moved
   to the slot list, leaving only non-TM variable dependencies
   on the variable list.
   (It looks like the chk_deps/chk_ldeps pair could be improved
   by getting all dependencies, then moving variable dependencies
   to slot dependencies once.)
*/
static void chk_deps(struct statpc *rule, struct statpc *dep) {
  struct deplst *vd, *va, *vb;

  /* first, prune vd list for redundancy building new list on va */
  va = NULL;
  for (vd = dep->u.dep.vd; vd != NULL; ) {
    if (name_test(vd->ref, NMTEST_TMDATUM))
      add_sdeps(dep, vd->ref->u.tmdecl->tmdef->sltcw, vd->flag);
    else if (name_test(vd->ref, NMTEST_VALID)) {
      if ((nr_validator(vd->ref)->flag & DCLF_VALIDATE)
          || (vd->flag & DEPL_ONCE)) {
        for (vb = va; vb != NULL && vb->ref != vd->ref; vb = vb->next);
        if (vb == NULL) { /* Must move to new list */
          vb = vd->next;
          vd->next = va;
          va = vd;
          vd = vb;
          continue;
        } else vb->flag |= vd->flag;
      }
    } else compile_error(4, "Unexpected NMTYPE %d in chk_deps", vd->ref->type);
    vb = vd->next;
    free_memory(vd);
    vd = vb;
  }
  dep->u.dep.vd = va;

  chk_ldeps(rule, dep);
}

/* Adjusts the slot position of a rule based on loose dependencies,
   i.e. dependencies inside 'depending on' statements.
*/
static void chk_loosely(unsigned int *per, unsigned int *row,
                        struct statpc *rule) {
  struct sltdep *sd;
  unsigned int n;
  
  for ( ; rule != NULL; rule = rule->next) {
    if (IS_DEPEND(rule->type)) {
      n = get_rate_period(&rule->u.dep.rate);
      if (n != 0) {
        add_sdep(rule, get_slot(n, 0), 0);
        rule->u.dep.rate.num = 0;
      }
      for (sd = rule->u.dep.sd; sd != NULL; sd = sd->next) {
        if (*per == 0) {
          compile_error(1, "TM dependency in independent statement");
          *per = sd->per; *row = sd->row;
          /* This is exactly the type of error where a line number
             would be very handy! */
          /* I make this a warning, because chk_loosely() is
             not supposed to affect the rate of an action.
             It might be that a better approach will simply
             leave the row and period 0 and attach the usual
             validation checks when the statement is executed,
             with the obvious problem being that the nested
             dependency may never be executed.
          */
        } else if (sd->per == *per) { if (sd->row > *row) *row = sd->row; }
        else if (sd->per < *per) {
          n = *row/sd->per;
          n = n * sd->per + sd->row;
          if (n > *row) *row = n;
        } else { /* sd->per > *per */
          if (sd->flag & DEPL_SPLIT)
            compile_error(3, "Sub-statement dependence on split datum");
          n = sd->row % *per;
          if (n > *row) *row = n;
        }
      }
      chk_loosely(per, row, rule->u.dep.stat.first);
      chk_loosely(per, row, rule->u.dep.else_stat.first);
    }
  }
}

/* Provides a new name for a validation status word. */
static char *get_vname(void) {
  static unsigned int vcnt = 0;
  static char vprefix[] = "V";
  char buf[20];
  
  sprintf(buf, "%s%d", vprefix, vcnt++);
  return(strdup(buf));
}

static struct valrec *newvrec(char *name, unsigned int bit) {
  struct valrec *vr;

  vr = new_memory(sizeof(struct valrec));
  vr->next = NULL;
  vr->varname = name;
  vr->bitval = bit;
  vr->funcname = NULL;
  return(vr);
}

/* Declares a status word and adds code to check it */
static void declare_vchk(unsigned int maxbit, char *name, unsigned int init,
                unsigned int reset, struct statpc *drule) {
  struct statpc *spc;
  struct valrec *vr;
  
  fprintf(ofile, "\nunsigned %s %s = 0x%X;",
                maxbit > 0x80 ? "int" : "char", name, init);
  vr = newvrec(name, reset);
  spc = drule->u.dep.stat.first;
  if (spc != NULL && spc->type == STATPC_VALCHK) {
    vr->next = spc->u.valchk;
    spc->u.valchk = vr;
  } else {
    spc = newstpc(STATPC_VALCHK);
    spc->u.valchk = vr;
    spc->next = drule->u.dep.stat.first;
    drule->u.dep.stat.first = spc;
  }
}

/* Generate validation records for outstanding variable and slot
   dependencies. A valrec defines a bit in a status word which is
   set/cleared when the slot/variable is in/validated.
   Variable valrecs are placed on the variable's declrtor.
   Slot valrecs are placed on the slot.
   declare_vchk() declares the status word and adds the STATPC_VALCHK
   to the beginning of the rule to test the validity.
*/
void gen_valrecs(struct statpc *drule) {
  unsigned int bit = 0, init, reset;
  char *vname;
  struct deplst *vd;
  struct sltdep *sd;
  struct valrec *vr;
  struct slt *slot;
  struct nm *ref;
  struct validator *vldtr;
  
  assert(IS_DEPEND(drule->type));
  vd = drule->u.dep.vd;
  sd = drule->u.dep.sd;
  while (vd != NULL || sd != NULL) {
    if (bit == 0x8000) {
      declare_vchk(bit, vname, init, reset, drule);
      bit = 0;
    }
    if (bit == 0) {
      bit = 1;
      init = reset = 0;
      vname = get_vname();
    } else bit <<= 1;
    vr = newvrec(vname, bit);
    if (vd != NULL) {
      if (vd->flag & DEPL_ONCE) reset |= bit;
      ref = vd->ref;
      assert(name_test(ref, NMTEST_VALID));
      vldtr = nr_validator(ref);
      if ((vldtr->flag & DCLF_TLSET) && !(vldtr->flag & DCLF_VALID))
        init |= bit;
      vr->next = vldtr->val;
      vldtr->val = vr;
      vd = vd->next;
    } else {
      assert(sd != NULL);
      if (sd->flag & DEPL_ONCE) reset |= bit;
      slot = get_slot(sd->per, sd->row);
      init |= bit;
      vr->next = slot->val;
      slot->val = vr;
      sd = sd->next;
    }
  }
  if (bit != 0)
    declare_vchk(bit, vname, init, reset, drule);
}

/* prune slot dependencies based on position, collection and synch
   and generate validation records for outstanding dependencies
  Build a new list:
    For the purposes of comparison, divide all periods and rows by
    SynchPer.
    Eliminate new slots if
        compatable with (contains) an existing slot
        compatable with (contains) action slot
        collection && period is less than or equal to action period
        if period is less than or equal to SynchPer
    Upgrade old slot if
        it is compatable with (contains) the new slot
*/
static void prune_slots(struct statpc *drule, struct sltdep *tslts,
                        unsigned int per, unsigned int row) {
  struct sltdep *sn, *nsl, *so;
  struct statpc *rule;
  unsigned int newlist;
  
  assert(IS_DEPEND(drule->type));
  nsl = NULL;
  for (sn = drule->u.dep.sd; sn != NULL; ) {
    if (!(
          /* compatable with action slot: */
          (sn->per <= per
            && ((row % sn->per)/SynchPer) == (sn->row/SynchPer))
          /* Collecting and period is <= action period */
          || (Collecting && sn->per <= per)
          /* Period <= SynchPer */
          || (sn->per <= SynchPer && !(sn->flag & DEPL_ONCE))
       )) {
      for (so = tslts, newlist = 0; ; so = so->next) {
        if (so == NULL && newlist == 0) {
          so = nsl;
          newlist = 1;
        }
        if (so == NULL) break;
        
        /* compare so (old) to sn (new) */
        if ( /* old contains new */
             sn->per <= so->per
             && ((so->row % sn->per)/SynchPer) == (sn->row/SynchPer)) {
          so->flag |= sn->flag;
          break;
        } else if ( /* new contains old */
                  newlist && sn->per > so->per
                  && ((sn->row % so->per)/SynchPer) == (so->row/SynchPer)) {
          so->per = sn->per;
          so->row = sn->row;
          so->flag |= sn->flag;
          break;
        }
      }
      if (so == NULL) {
        /* copy sn into the new slot */
        so = sn->next;
        sn->next = nsl;
        nsl = sn;
        sn = so;
        continue;
      }
    }
    /* Discard new slot */
    so = sn->next;
    free_memory(sn);
    sn = so;
  }
  drule->u.dep.sd = nsl;
  
  gen_valrecs(drule);
  
  /* Now recurse to do lower depending ons */
  if (tslts != NULL) nsl = tslts;
  for (rule = drule->u.dep.stat.first; rule != NULL; rule = rule->next) {
    if (IS_DEPEND(rule->type))
      prune_slots(rule, nsl, per, row);
  }
  for (rule = drule->u.dep.else_stat.first; rule != NULL; rule = rule->next) {
    if (IS_DEPEND(rule->type))
      prune_slots(rule, nsl, per, row);
  }
}

/* place_indep() handles top-level statements which have no slot
   dependence. There are three cases:
     No variable dependence: Error
     Dependence on one variable:
       place statement directly on variable's declrtor and
       remove from variable dependence
     Dependence on more than one variable:
       place common statement on each variable's declrtor and
       leave dependence so validation records will be generated
       by prune_slots/gen_valrecs
*/
void place_indep(struct statpc *rule) {
  struct nm *ref;
  struct deplst *vd;
  struct validator *vldtr;

  assert(rule != NULL
         && IS_DEPEND(rule->type)
         && rule->next == NULL);
  if (rule->u.dep.vd == NULL) {
    FILE *oofile;
    
    compile_error(2, "Statement has no dependency:");
    oofile = ofile;
    ofile = stderr;
    print_stat(rule);
    ofile = oofile;
  } else if (rule->u.dep.vd->next == NULL) {
    ref = rule->u.dep.vd->ref;
    vldtr = nr_validator(ref);
    catstatpc(&vldtr->valstat, rule);
    free_memory(rule->u.dep.vd);
    rule->u.dep.vd = NULL;
  } else for (vd = rule->u.dep.vd; vd != NULL; vd = vd->next) {
    ref = vd->ref;
    vldtr = nr_validator(ref);
    catstatpc(&vldtr->valstat, common_stat(rule));
  }
}

/* Top-level check dependencies routine. Calls the recursive chk_deps,
   then determines position based on slotlist, adjusts position
   based on nested dependencies and places the rule on the appropriate
   slot.
*/
static void chk_tdeps(struct statpc *rule) {
  unsigned int per, row, n;
  struct sltdep *sd;
  struct slt *slot;
  struct statpc *spc;

  /* check dependencies */
  chk_deps(rule->u.dep.stat.first, rule);
  chk_deps(rule->u.dep.else_stat.first, rule);

  /* Determine position based on slotlist */
  row = 0;
  per = get_rate_period(&rule->u.dep.rate);
  for (sd = rule->u.dep.sd; sd != NULL; sd = sd->next) {
    if (per == 0) { per = sd->per; row = sd->row; }
    else if (sd->per == per) { if (sd->row > row) row = sd->row; }
    else if (sd->per < per) {
      n = row/sd->per;
      n = n * sd->per + sd->row;
      if (n > row) row = n;
    } else { /* sd->per > per */
      n = sd->row / per;
      row = n * per + row;
      per = sd->per;
      if (sd->row > row) row = sd->row;
    }
  }

  /* Adjust position loosely based on any constituent depending ons */
  chk_loosely(&per, &row, rule->u.dep.stat.first);
  chk_loosely(&per, &row, rule->u.dep.else_stat.first);

  /* Handle per 0 data */
  if (per == 0) place_indep(rule);

  /* Remove redundant dependencies from lists and generate validations
     for outstanding slots */
  prune_slots(rule, NULL, per, row);

  /* Now that we're positioned, Check references for global positioning */
  slot = get_slot(per, row);
  chk_refs(slot, rule);
  
  /* place rule on the designated slot */
  if (slot != NULL) {
    spc = newstpc(STATPC_TEXT);
    spc->u.text = NULL;
    catstatpc(&slot->extacts, spc);
    catstatpc(&slot->extacts, rule);
  }
}

/* Place all extraction statements on appropriate slots.
   Calls chk_tdeps to do most of the work.
*/
void place_ext(void) {
  struct statpc *spc;

  for (spc = program; spc != NULL; spc = spc->next) {
    switch (spc->type) {
      case STATPC_EXTRACT:
        assert(spc->u.stat.first != NULL
               && IS_DEPEND(spc->u.stat.first->type));
        chk_tdeps(spc->u.stat.first);
        break;
      case STATPC_VALID:
      case STATPC_INVALID:
      case STATPC_TLDECLS:
        break;
      default:
        compile_error(4, "Unexpected STATPC type %d in place_valid", spc->type);
        break;
    }
  }
}

struct cwsort {
  struct cwsort *next;
  struct cw *scw;
  struct nm *nameref;
  unsigned int placed;
};

/* Sort by increasing columns */
static struct cwsort *sort_cw(struct cwsort *list, struct cw *cwl,
                              struct nm *name) {
  struct cwsort *cwb, *cwc, *ncw;

  ncw = new_memory(sizeof(struct cwsort));
  ncw->scw = cwl;
  ncw->nameref = name;
  ncw->placed = 0;
  for (cwb = NULL, cwc = list;
       cwc != NULL && ncw->scw->col > cwc->scw->col;
       cwb = cwc, cwc = cwc->next);
  ncw->next = cwc;
  if (cwb == NULL) return(ncw);
  cwb->next = ncw;
  return(list);
}

static void output_member(struct cwsort *ref, unsigned int *offset,
                          unsigned int snum, unsigned int *unum) {
  struct declrtor *decl;
  struct tmalloc *tma;
  char buf[40];

  if (ref->scw->col > *offset) {
    print_indent("\nchar ");
    fprintf(ofile, "U%d[%d];", (*unum)++, ref->scw->col - (*offset));
    *offset = ref->scw->col;
  }
  /* Basic assumption seems to be that these are TMDATUMs */
  assert(name_test(ref->nameref, NMTEST_TMDATUM));
  decl = nr_declarator(ref->nameref);
  tma = ref->nameref->u.tmdecl->tmdef;
  if (tma->sltcw->dnext == NULL) { /* single-slot data */
    adjust_indent(0);
    print_stat(decl->typeparts.first);
    print_stat(decl->decl.first);
    sprintf(buf, "home->U%d.%s", snum, ref->nameref->name);
  } else { /* multi-slot data: treat as char [width] */
    print_indent("\nchar ");
    fprintf(ofile, "U%d[%d]", *unum, ref->scw->width);
    sprintf(buf, "home->U%d.U%d", snum, (*unum)++);
  }
  *offset += ref->scw->width;
  ref->placed = 1;
  fputc(';', ofile);
  ref->scw->home_row_text = strdup(buf);
}

void place_home(void) {
  struct nm *name;
  struct cw *cwl;
  struct cwsort base, *sla;
  unsigned int unum = 0, offset, snum;
  
  base.next = NULL;
  base.scw = NULL;
  base.nameref = NULL;
  base.placed = 0;
  for (name = global_scope->names; name != NULL; name = name->next) {
    if (name->type == NMTYPE_TMDATUM)
      for (cwl = name->u.tmdecl->tmdef->sltcw; cwl != NULL;
           cwl = cwl->dnext)
        base.next = sort_cw(base.next, cwl, name);
  }
  adjust_indent(0);
  print_indent("union {");
  adjust_indent(2);
  for (;;) {
    offset = TM_Data_Type == 3 ? 2 : 0;
    for (sla = base.next;
         sla != NULL && (sla->placed != 0 || sla->scw->col < offset);
         sla = sla->next);
    if (sla == NULL) break;
    snum = unum++;
    print_indent("\nstruct {");
    adjust_indent(2);
    while (sla != NULL) {
      output_member(sla, &offset, snum, &unum);
      for (sla = sla->next;
           sla != NULL && (sla->placed != 0 || sla->scw->col < offset);
           sla = sla->next);
    }
    adjust_indent(-2);
    print_indent(NULL);
    fprintf(ofile, "} U%d;", snum);
  }
  adjust_indent(-2);
  print_indent("} *home;\n");
  while (base.next != NULL) {
    sla = base.next;
    base.next = sla->next;
    free_memory(sla);
  }
}
