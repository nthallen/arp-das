/* pfuncs.c
   Contains routines to output function definitions.
   Revision 1.4  2008/07/16 19:13:40  ntallen
   Compiling support for TM_Data_Type 3
   Omit definitions for Synch and MFCtr from home row

   Revision 1.3  2008/07/16 18:55:14  ntallen
   Changes to support TM_Data_Type 3

   Revision 1.2  2008/07/03 18:18:48  ntallen
   To compile under QNX6 with minor blind adaptations to changes between
   dbr.h and tm.h

   Revision 1.1  2008/07/03 15:11:07  ntallen
   Copied from QNX4 version V1R9

   Revision 1.10  1993/09/27 19:38:07  nort
   Cleanup.

 * Revision 1.9  1993/06/23  17:24:45  nort
 * Corrected bug when printing nested IV functions: printf w/o args
 * One for lint.
*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "rational.h"
#include "tmcstr.h"
#include "tmc.h"

#define NULLFUNCDECL "\nstatic void " NULLFUNCNAME "(void)"
#define MVSLOTS_IN 0
#define MVSLOTS_OUT 1

/* Produce code to move data in/out of the homerow structure */
static void moveslots(struct cw *cwl, unsigned int dir) {
  struct nm *datum;
  struct cw *cwn;
  unsigned int offset;
  struct tmalloc *tma;
  
  assert(TM_Data_Type > 0);
  for (; cwl != NULL; cwl = cwl->next) {
    datum = cwl->datum;
    if (name_test(datum, NMTEST_TMDATUM)) {
      tma = nr_tmalloc(datum);
      if (!(tma->flags & TMDF_HOMEROW)) {
        cwn = tma->sltcw;
        /* Simple assignment is allowed if the datum
         * is contiguous in the homerow (i.e. not split
         * over more than one slot) and not an array
         * type.
         */
        if (cwn == cwl && cwl->dnext == NULL &&
            (datum->u.tmdecl->decl->flag & DCLF_ARRAY) == 0) {
          /* Can move with simple assignment */
          if ( TM_Data_Type != 3 || ( cwl->col >= 2 && cwl->col < Ncols - 2) ) {
            /* Type 3 suppresses MFCtr and Synch columns, so no point copying them */
            assert(cwl->home_row_text != NULL);
            if (dir == MVSLOTS_IN)
              fprintf(ofile, "\n  %s = %s;", cwl->home_row_text, datum->name);
            else
              fprintf(ofile, "\n  %s = %s;", datum->name, cwl->home_row_text);
          }
        } else {
          assert(cwl->home_row_text != NULL);
          for (offset = 0; cwn != cwl; cwn = cwn->dnext) {
            assert(cwn != NULL);
            offset += cwn->width;
          }
          /* must move via memcpy */
          fprintf(ofile, "\n  memcpy(");
          if (dir == MVSLOTS_IN) fprintf(ofile, "%s, ", cwl->home_row_text);
          if (offset)
            fprintf(ofile, "((char*)(&%s))+%d, ", datum->name, offset);
          else fprintf(ofile, "&%s, ", datum->name);
          if (dir != MVSLOTS_IN) fprintf(ofile, "%s, ", cwl->home_row_text);
          fprintf(ofile, "%d);", cwl->width);
        }
      }
    }
  }
}

/* Print statements to in/validate a slot/variable */
void print_valid(struct valrec *val, int valid) {
  struct validator vldtr;
  
  vldtr.flag = 0;
  vldtr.val = val;
  initstat(&vldtr.valstat, NULL);
  print_vldtr(&vldtr, valid);
}

void print_vldtr(struct validator *vldtr, int valid) {
  struct valrec *val;
  int use_parens;

  if (vldtr == NULL) return;
  use_parens = (valid && vldtr->valstat.first != NULL)
    || (vldtr->val != NULL && vldtr->val->next != NULL);
  if (use_parens) {
    fprintf(ofile, " {");
    adjust_indent(2);
  }
  for (val = vldtr->val; val != NULL; val = val->next) {
    if (val->varname != NULL) {
      print_indent(NULL);
      fprintf(ofile, "%s %s0x%X;", val->varname,
              valid ? "&= ~" : "|= ", val->bitval);
      adjust_indent(0);
    } else {
      assert(val->funcname != NULL);
      if (valid) {
        print_indent(NULL);
        fprintf(ofile, "%s;", val->funcname);
        adjust_indent(0);
      }
    }
  }
  if (valid) print_stat(vldtr->valstat.first);
  if (use_parens) {
    adjust_indent(-2);
    print_indent("}");
    adjust_indent(0);
  }
}

#define GF_COLLECT 1
#define GF_EXTRACT 2
#define GF_BOTH (GF_COLLECT|GF_EXTRACT)
#define GF_PBOTH (4|GF_BOTH)

/* gen_func generates the designated function for the designated slot
   if it has not already been generated.
    Optimize: {
      A function which does nothing should not be generated
      A function which only calls one other function should not
      be generated, but simply reference the other function.
      A function which does nothing but call two other functions
      may be a candidate for combination with other functions
    }
*/
static void chk_func(struct slt *slot, unsigned int which) {
  struct cw *cwl;
  struct nm *datum;
  unsigned int bndd = 0;

  if (slot == NULL) return;
  if ((slot->flag & (SLT_CCHKD|SLT_ECHKD|SLT_BCHKD|SLT_PCHKD)) == 0) {
    /* Check for NTHR */
    /* Check for non-home-row cw's */ 
    for (cwl = slot->cwl; cwl != NULL; cwl = cwl->next) {
      datum = cwl->datum;
      if (name_test(datum, NMTEST_TMDATUM)
          && !(nr_tmalloc(datum)->flags & TMDF_HOMEROW)) break;
    }
    if (cwl != NULL) slot->flag |= SLT_NTHR;
  }

  /* Have we already checked? */
  switch (which) {
    case GF_COLLECT:
      if (slot->flag & SLT_CCHKD) return;
      slot->flag |= SLT_CCHKD;
      if (Collecting && (slot->colacts.first != NULL
            || (slot->flag & SLT_NTHR)))
        slot->flag |= SLT_CNDD;
      break;
    case GF_EXTRACT:
      if (slot->flag & SLT_ECHKD) return;
      slot->flag |= SLT_ECHKD;
      if (slot->val != NULL || slot->extacts.first != NULL
          || (!Collecting && (slot->flag & SLT_NTHR)))
        slot->flag |= SLT_ENDD;
      break;
    case GF_BOTH:
      if (slot->flag & SLT_BCHKD) return;
      slot->flag |= SLT_BCHKD;
      if ((Collecting && slot->colacts.first != NULL)
          || (slot->flag & SLT_NTHR)
          || slot->val != NULL
          || slot->extacts.first != NULL
         ) {
        slot->flag |= SLT_BNDD;
        bndd = 1;
      }
      break;
    case GF_PBOTH:
      if (slot->flag & SLT_PCHKD) return;
      slot->flag |= SLT_PCHKD;
      if (Collecting && (slot->colacts.first != NULL
                          || (slot->flag & SLT_NTHR)))
        slot->flag |= SLT_CNDD;
      if ((!Collecting && (slot->flag & SLT_NTHR))
          || slot->val != NULL
          || slot->extacts.first != NULL
         ) {
        slot->flag |= SLT_ENDD;
        bndd = 1;
      }
      break;
  }
  if (bndd) {
    if ((Collecting && slot->colacts.first != NULL)
        /* We have any collection actions to print */
        || (slot->val != NULL)
        /* We have validations to print */
        || (slot->flag & SLT_NTHR)
        /* We have non-trivial HR */
        ) {
      chk_func(slot->calls, GF_COLLECT);
      chk_func(slot->calls, GF_EXTRACT);
    } else chk_func(slot->calls, GF_PBOTH);
  } else chk_func(slot->calls, which);
}

/* When generating GF_COLLECT and GF_EXTRACT are pretty straightforward.
   For GF_BOTH, if either C or E has been checked, then the B func
   should just call the C and E funcs (even if they are not NDD).
   If neither is checked, then the B func can do the whole nine yards.
*/
static void gf(struct slt *slot, unsigned int which) {
  char **fname, prefix, buf[20];

  switch (which) {
    case GF_COLLECT:
      if (!(slot->flag & SLT_CNDD)) {
        if (slot->calls != NULL && (slot->flag & SLT_CCHKD))
          slot->cfunc = slot->calls->cfunc;
        return;
      }
      prefix = 'C';
      fname = &slot->cfunc;
      break;
    case GF_EXTRACT:
      if (!(slot->flag & SLT_ENDD)) {
        if (slot->calls != NULL && (slot->flag & SLT_ECHKD))
          slot->efunc = slot->calls->efunc;
        return;
      }
      prefix = 'E';
      fname = &slot->efunc;
      break;
    case GF_BOTH:
      if (!(slot->flag & SLT_BNDD)) {
        if (slot->calls != NULL && (slot->flag & SLT_BCHKD))
          slot->bfunc = slot->calls->bfunc;
        return;
      } else if (slot->flag & (SLT_CCHKD|SLT_ECHKD)) {
        if (slot->cfunc == NULL) {
          slot->bfunc = slot->efunc;
          return;
        } else if (slot->efunc == NULL) {
          slot->bfunc = slot->cfunc;
          return;
        }
      }
      prefix = 'B';
      fname = &slot->bfunc;
      break;
  }
  sprintf(buf, "%cF%d_%d", prefix, slot->per, slot->row);
  *fname = strdup(buf);
  fprintf(ofile, "\nstatic void %s(void) {", *fname);
  adjust_indent(2);
  if (which == GF_BOTH && (slot->flag & (SLT_CCHKD|SLT_ECHKD))) {
    assert(slot->cfunc != NULL && slot->efunc != NULL);
    print_indent(NULL);
    fprintf(ofile, "%s();\n  %s();", slot->cfunc, slot->efunc);
  } else {
    if (which == GF_BOTH && slot->calls != NULL
        && (!(slot->calls->flag & (SLT_CCHKD|SLT_ECHKD)))
        && ((!Collecting) || slot->colacts.first == NULL)
          /* We have no collection actions to print */
        && (slot->val == NULL)
          /* We have no validations to print */
        && (!(slot->flag & SLT_NTHR))
          /* We don't have non-trivial HR */
        ) {
      if (slot->calls->bfunc != NULL) {
        print_indent(NULL);
        fprintf(ofile, "%s();", slot->calls->bfunc);
      }
    } else {
      if (Collecting && (which & GF_COLLECT)) {
        if (slot->calls) {
          if (slot->calls->cfunc != NULL) {
            print_indent(NULL);
            fprintf(ofile, "%s();", slot->calls->cfunc);
            adjust_indent(0);
          }
        }
        print_stat(slot->colacts.first);
        moveslots(slot->cwl, MVSLOTS_IN);
      }
      if (which & GF_EXTRACT) {
        if (!Collecting) moveslots(slot->cwl, MVSLOTS_OUT);
        print_valid(slot->val, 1);
        if (slot->calls && slot->calls->efunc != NULL) {
          adjust_indent(0);
          print_indent(NULL);
          fprintf(ofile, "%s();", slot->calls->efunc);
          adjust_indent(0);
        }
      }
    }
    if (which & GF_EXTRACT) print_stat(slot->extacts.first);
  }
  adjust_indent(-2);
  print_indent("}\n");
}

static void gen_func(struct slt *slot) {
  if (slot == NULL || slot->flag & SLT_GEND) return;
  slot->flag |= SLT_GEND;
  gen_func(slot->calls);
  if (slot->flag & SLT_PCHKD) {
    slot->flag &= ~SLT_PCHKD;
    if (slot->flag & (SLT_CCHKD|SLT_ECHKD))
      slot->flag |= SLT_CCHKD|SLT_ECHKD;
    else {
      slot->flag |= SLT_BCHKD;
      if (slot->flag & (SLT_CNDD|SLT_ENDD)) {
        slot->flag &= ~(SLT_CNDD|SLT_ENDD);
        slot->flag |= SLT_BNDD;
      }
    }
  }
  gf(slot, GF_COLLECT);
  gf(slot, GF_EXTRACT);
  gf(slot, GF_BOTH);
}

/* Generate invalidation functions IVF_*() to be called when
   rows are missed. Returns 1 if a function was generated, 0
   otherwise. First generate 'calls' functions recursively.
   If no 'calls' functions generated, and we don't have
   any validations, nothing to do.
*/
int gen_ivfunc(struct slt *slot) {
  int has_calls;
  
  if (slot == NULL) return(0);
  has_calls = gen_ivfunc(slot->calls);
  if (slot->val == NULL) {
    if (has_calls) slot->ivfunc = slot->calls->ivfunc;
    else return(0);
  } else if (slot->ivfunc == NULL) {
    char buf[20];
    
    sprintf(buf, "IVF%d_%d", slot->per, slot->row);
    slot->ivfunc = strdup(buf);
    fprintf(ofile, "\nstatic void %s(void) {", buf);
    adjust_indent(2);
    if (has_calls) {
      print_indent("\n");
      fprintf(ofile, "%s();", slot->calls->ivfunc);
    }
    print_valid(slot->val, 0);
    adjust_indent(-2);
    print_indent("\n}");
  }
  return(1);
}

/* gen_ivfuncs Generates invalidation functions if necessary,
   including the ivfuncs[] array. Returns TRUE if the nullfunc
   is required.
*/
static int gen_ivfuncs(void) {
  if (!Collecting) {
    unsigned int per, row;
    struct slt *slot;
    int has_ivfs = 0;

    per = sltlist->per;
    for (row = 0; row < per; row++) {
      slot = get_slot(per, row);
      if (gen_ivfunc(slot)) has_ivfs = 1;
    }
    if (has_ivfs) {
      fprintf(ofile, "\n#define IVFUNCS");
      fprintf(ofile, "\nstatic void (*ivfuncs[%d])() = {\n  ", per);
      for (has_ivfs = 0, row = 0; row < per; row++) {
        if (row != 0) fprintf(ofile, ",\n  ");
        slot = get_slot(per, row);
        if (slot->ivfunc != NULL) fprintf(ofile, "%s", slot->ivfunc);
        else {
          fprintf(ofile, NULLFUNCNAME);
          has_ivfs = 1;
        }
      }
      fprintf(ofile, "\n};\n");
      return(has_ivfs);
    }
  }
  return(0);
}

void print_funcs(void) {
  unsigned int per, row;
  struct slt *slot;
  int need_nfunc;
  
  /* output tminitfunc always */
  fprintf(ofile, "\nvoid tminitfunc(void) {");
  if (initprog.first != NULL) {
    adjust_indent(2);
    print_stat(initprog.first);
    adjust_indent(-2);
  }
  print_indent("}\n");
  
  /* output a null function declaration */
  fprintf(ofile, NULLFUNCDECL ";");
  
  assert(sltlist != NULL);
  per = sltlist->per;
  for (row = 0; row < per; row++) {
    slot = get_slot(per, row);
    chk_func(slot, GF_BOTH);
  }
  for (row = 0; row < per; row++) {
    slot = get_slot(per, row);
    gen_func(slot);
  }
  fprintf(ofile, "\nstatic void (*efuncs[%d])() = {\n  ", per);
  for (need_nfunc = 0, row = 0; row < per; row++) {
    if (row != 0) fprintf(ofile, ",\n  ");
    slot = get_slot(per, row);
    if (slot->bfunc != NULL) fprintf(ofile, "%s", slot->bfunc);
    else {
      fprintf(ofile, NULLFUNCNAME);
      need_nfunc = 1;
    }
  }
  fprintf(ofile, "\n};\n");
  if (gen_ivfuncs() || need_nfunc)
    fprintf(ofile, NULLFUNCDECL "{}");
}
