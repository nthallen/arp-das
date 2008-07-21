/* genpcm.c Generates PCM from TM data definitions
   $Log$
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

   Revision 1.10  1996/04/18 19:08:05  nort
   Changes to better control frame dimensions:
   TM MINCOLS=<n>; and explicit specification of Synch rate.

 * Revision 1.9  1995/10/26  03:13:07  nort
 * Got more tolerant when printing statistics; needs more work.
 *
 * Revision 1.8  1995/10/26  02:53:51  nort
 * Remove another assert failure doing unnecessary rational
 * arithmetic. ACATS pushed us over the edge.
 *
 * Revision 1.7  1995/10/18  02:02:55  nort
 * *** empty log message ***
 *
 * Revision 1.6  1993/09/27  19:36:44  nort
 * Changes for common compiler functions, cleanup of rlog, pragmas.
 *
 * Revision 1.5  1993/04/01  22:03:21  nort
 * Restructuring
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

unsigned int show_tm = 0;

unsigned int BPMFmax = 64, BPMFmin = 0, Nrows, Ncols, Synchronous = 0;
unsigned int SynchPer; /* Rows per Minor Frame */
unsigned long int SynchTolerance = 0L;
long int SynchDrift; /* msecs/hour */
unsigned int SynchValue = 0xB4AB;
unsigned int SynchInverted = 0;
unsigned int SecondsDrift = 90;
int TM_Data_Type = 0;
rational Rsynch;

/* tmcompare(a,b) returns an integer which is:
   > 0 if a precedes b
     0 if a equals b
   < 0 if b precedes a
   The order used is:
     Faster rates precede slower rates
	 else larger data precedes smaller data
	 else alphabetical order
*/
static int tmcompare(struct tmalloc *a, struct tmalloc *b) {
  int rv;
  
  rv = rcompare(&a->rate, &b->rate);
  if (rv == 0) rv = (int) a->size - (int) b->size;
  if (rv == 0) rv = stricmp(b->nameref->name, a->nameref->name);
  return(rv);
}

/* tmmerge merges the new element nrt into the list, sorting on the
   function tmcompare()
 */
struct tmalloc *tmmerge(struct tmalloc *list, struct tmalloc *nrt) {
  struct tmalloc *pos, *ppos; /* first and last */

  assert(nrt != NULL && nrt->next == NULL);
  if (list == NULL) return(nrt);
  if (tmcompare(nrt, list) > 0) {
	nrt->next = list;
	return(nrt);
  } else for (ppos = list;; ppos = pos) {
    pos = ppos->next;
	if (pos == NULL) {
	  ppos->next = nrt;
	  return(list);
	}
	if (tmcompare(nrt, pos) > 0) {
	  nrt->next = pos;
	  ppos->next = nrt;
	  return(list);
	}
  }
}

struct slt *sltlist = NULL;
static void clear_sltlist(void) {
  struct cw *cwl, *ocw;
  struct slt *osl;
  
  while (sltlist != NULL) {
    for (cwl = sltlist->cwl; cwl != NULL; ) {
	  ocw = cwl;
	  cwl = ocw->next;
	  free_memory(ocw);
	}
	osl = sltlist;
	sltlist = sltlist->next;
	free_memory(osl);
  }
}

static struct cw *add_cw(struct cw *cwl, struct cw *ncw) {
  struct cw *cwb, *cwc;

  for (cwb = NULL, cwc = cwl;
	   cwc != NULL && cwc->slot->row < ncw->slot->row;
	   cwb = cwc, cwc = cwc->dnext);
  ncw->dnext = cwc;
  if (cwb == NULL) return(ncw);
  cwb->dnext = ncw;
  return(cwl);
}

/* Returns non-zero if the specified slot is available.
   If slide is non-zero, the column can be freely chosen
   to locate a free position.
   
   sltlist is in decreasing period order.
   Start at the top.
   Let p',r' be the period and row of the current slot.
   if (p' >= p && r'%p == r) check cwlist for collisions
   if (p' <  p && r%p' == r') check cwlist for collisions
   if (p' <= p) follow "calls" pointer
   else follow next. 
*/
static int find_col(unsigned int p, unsigned int r,
             unsigned int *c, unsigned int w, int slide) {
  struct slt *sl, *ts;
  struct cw *cwl;
  int moved;

  ts = NULL;
  moved = 0;
  if (slide) *c = 0;
  for (sl = NULL; ; ) {
	if (moved) {
	  moved = 0;
	  sl = ts;
	} else if (sl != NULL && sl->per <= p)
	  sl = sl->calls;
	else for (sl = (sl!=NULL) ? sl->next : sltlist; sl != NULL; sl = sl->next)
	  if ((sl->per <= p && (r % sl->per) == sl->row) ||
		  (sl->per > p && (sl->row % p) == r))
		break;
	if (sl == NULL) return(1); /* no collisions == success */
	if (ts == NULL) ts = sl; /* first compatable row */
	for (cwl = sl->cwl; cwl != NULL; cwl = cwl->next) {
	  if (cwl->col < *c + w && cwl->col + cwl->width > *c) {
		if (slide) {
		  *c = cwl->col + cwl->width;
		  if (*c + w > Ncols) return(0);
		  moved = 1;
		  break; /* go back to the top */
		} else return(0);
	  }
	}
  }  
}

/* Returns non-zero if a slot of period p and width w is available.
   Calls find_col.
*/   
static int find_slot(unsigned int p, unsigned int w,
			  unsigned int *r, unsigned int *c) {
  for (*r = 0; *r < p; ++(*r))
	if (find_col(p, *r, c, w, 1)) return(1);
  return(0);
}

/* Finds a slot with period <= p which is compatable with (p,r).
   If slot is not NULL, starts looking at slot->next. Otherwise, starts
   from the top of the sltlist.
*/   
struct slt *comp_slot(struct slt *sl, unsigned int p, unsigned int r) {
  if (sl == NULL) sl = sltlist;
  else sl = sl->next;
  for ( ; sl != NULL; sl = sl->next)
	if (sl->per <= p && (r % sl->per) == sl->row) break;
  return(sl);
}

/* get_slot finds or creates a specific slot. Called from
   take_slot as well as place.c. Returns NULL if period is
   0.
*/
struct slt *get_slot(unsigned int p, unsigned int r) {
  struct slt *sl, *slnxt, *slnew;

  if (p == 0) return(NULL);
  for (sl = NULL, slnxt = sltlist; ; sl = slnxt, slnxt = slnxt->next) {
	if (slnxt != NULL) {
	  if (slnxt->per > p) continue;
	  if (slnxt->per == p) {
		if (slnxt->row == r) return(slnxt);
		if (slnxt->row > r) continue;
	  }
	}
	/* insert a new slt here */
	slnew = new_memory(sizeof(struct slt));
	slnew->next = slnxt;
	slnew->per = p;
	slnew->row = r;
	slnew->cwl = NULL;
	initstat(&slnew->colacts, NULL);
	initstat(&slnew->extacts, NULL);
	slnew->val = NULL;
	slnew->cfunc = slnew->efunc = slnew->bfunc = slnew->ivfunc = NULL;
	slnew->flag = 0;
	if (sl == NULL) sltlist = slnew;
	else sl->next = slnew;
	
	/* fill in the calls fields */
	for (sl = sltlist; sl != NULL && sl->per > p; sl = sl->next) {
	  slnxt = comp_slot(sl, sl->per, sl->row);
	  if (slnxt == slnew) sl->calls = slnew;
	}
	slnew->calls = comp_slot(slnew, p, r);
	return(slnew);
  }
}

/* Claims a slot (assumed available) in the name of the specified datum */
static struct cw *take_slot(unsigned int p, unsigned int r,
             unsigned int c, unsigned int w) {
  struct slt *sl;
  struct cw *cwn;

  sl = get_slot(p, r);
  /* Now add a cw record to this slot */
  cwn = new_memory(sizeof(struct cw));
  cwn->slot = sl;
  cwn->next = sl->cwl;
  cwn->dnext = NULL;
  cwn->col = c;
  cwn->width = w;
  cwn->home_row_text = NULL;
  sl->cwl = cwn;
  return(cwn);
}

static struct tmalloc *synsort, *mfrsort;

/* position_data does the actual positioning of the data */
static int position_data(struct tmalloc *list, rational *Rrow) {
  rational rat;
  unsigned int p, r, c, w, w1;
  
  /* Clear any existing definitions */
  clear_sltlist();
  
  /* Add definitions for Synch and Minfr */
  c = Ncols -2;
  if (find_col(SynchPer, SynchPer - 1, &c, 2, 0) == 0)
	compile_error(4, "Synch could not be placed!");
  synsort->sltcw = take_slot(SynchPer, SynchPer - 1, c, 2);
  c = (SynchPer > 1) ? Ncols-2: 0;
  if (find_col(SynchPer, 0, &c, 2, 0) == 0)
    compile_error(4, "MinFr could not be placed!");
  mfrsort->sltcw = take_slot(SynchPer, 0, c, 2);
  
  /* Add definitions for the rest */
  for (; list != NULL; list = list->next) {
	rdivide(Rrow, &list->rate, &rat);
	assert(rat.den == 1);
	p = rat.num;
	w1 = w = list->size;
	list->sltcw = NULL;
	while (w > 0) {
	  for (w1 = w; w1>0 && !find_slot(p, w1, &r, &c) ; w1--) ;
	  if (w1 == 0) return(0); /* couldn't find anything of period p */
	  list->sltcw = add_cw(list->sltcw, take_slot(p, r, c, w1));
	  w -= w1;
	}
  }
  return(1);
}

/* pos_rate is given Rsyn, the rate of Synch (or MF) and attempts to
   calculate the frame. It will adjust Ncols if necessary to fit all
   the data into a frame.
   returns zero on success

   Rrow = max(Rsyn, Rmax)
   Rmaj = min(Rsyn, Rmin)
   Nrows = Rrow/Rmaj
   BPS = DBPS + 4Rsyn
   Ncols = ceil(BPS/Rrow)
   adjust ncols up as necessary to achieve acceptable error rate if synch.
   but not so BPMF exceeds BPMFmax. else Rsyn = Rrow*2;
*/
static int pos_rate(struct tmalloc *list, rational *Rsyn,
				rational *Rmax, rational *Rmin, rational *DBPS) {
  rational BPS, rtemp, Rrow, Rmaj;
  unsigned int BPMF; /* bytes per MF */
  
  /* Calculate Nrows */
  Rrow = (rcompare(Rsyn, Rmax) >= 0) ? *Rsyn : *Rmax;
  Rmaj = (rcompare(Rsyn, Rmin) <= 0) ? *Rsyn : *Rmin;
  rdivide(&Rrow, &Rmaj, &rtemp);
  assert(rtemp.den == 1);
  Nrows = rtemp.num;

  /* Calculate Ncols */  
  rdivide(&Rrow, Rsyn, &BPS);
  assert(BPS.den == 1);
  SynchPer = BPS.num;
  rtimesint(Rsyn, 4, &BPS);
  rplus(&BPS, DBPS, &BPS);
  rdivide(&BPS, &Rrow, &rtemp);
  Ncols = (rtemp.num + rtemp.den - 1)/rtemp.den;
  for (;; Ncols++) {
    BPMF = SynchPer * Ncols;
	if (BPMF < BPMFmin) continue;
	if (BPMF > BPMFmax) return(1);
	if (Synchronous) {
	  rational BitsPS;
	  long int clkdiv, ldiff, lden;
	  
	  rtimesint(&Rrow, Ncols*8, &BitsPS);
	  rtimesint(&BitsPS, 2, &BitsPS); /* x2 clock */
	  clkdiv = 1048576L * BitsPS.den;
	  clkdiv /= BitsPS.num;
	  /* clkdiv is what we program the 8251 with for synch trans */
	  if (clkdiv > 0x10000L) {
		compile_error(1, "Bit Rate too slow for synchronous collection");
	    return(1);
	  }
	  /* The resulting rate R' = 2^20/clkdiv. Target rate is R = BitsPS.
	     Calculate (R'-R)/R
	  */
	  lden = BitsPS.num * clkdiv;
	  ldiff = BitsPS.den * 1048576L - lden;
	  if (ldiff != 0) {
		if (SynchTolerance == 0) continue;
		SynchDrift = ldiff * SynchTolerance;
		if (SynchDrift > lden || (-SynchDrift) > lden) continue;
	  }
	  SynchDrift = ldiff * 3600000L;
	  SynchDrift /= lden;
	}
	if (show(TM_STEPS)) {
	  fprintf(vfile,
	    "Attempting frame with %d/%d rows/sec, %d RPMF, %d x %d\n",
		Rrow.num, Rrow.den, SynchPer, Nrows, Ncols);
	}
	if (position_data(list, &Rrow)) return(0);
  }
}

static struct tmalloc *synmfr(char *name) {
  struct nm *datum;

  datum = find_name(name, 0);
  if (datum == NULL)
	compile_error(3, "%s not defined", name);
  if (!name_test(datum, NMTEST_TMDATUM))
	compile_error(3, "%s must be defined as TM datum", datum->name);
  return(datum->u.tmdecl->tmdef);
}

/* next_rate increases Rsyn to the next legal rate. If ge is non-zero
   and Rsyn is already a legal rate, it will not be changed. Otherwise
   it will always be increased.
*/   
static void next_rate(struct tmalloc *list, rational *Rsyn, int ge) {
  rational Ra, rtemp;
  int compval;
  unsigned int intratio, divider;

  if (rcompare(Rsyn, &list->rate) >= 0) {
	/* If Rsyn >= Rmax, adjust Rsyn = Rmax * ceil(Rsyn/Rmax)
	   (i.e. round up to the nearest multiple of Rmax)
	*/
	rdivide(Rsyn, &list->rate, Rsyn);
	if (ge) Rsyn->num = (Rsyn->num + Rsyn->den - 1)/Rsyn->den;
	else Rsyn->num = Rsyn->num/Rsyn->den + 1;
	Rsyn->den = 1;
	rtimes(Rsyn, &list->rate, Rsyn);
  } else { /* Rsyn < list->rate */
	for (; list != NULL; list = list->next) {
	  compval = rcompare(&list->rate, Rsyn);
	  if (compval <= 0) {
		if (compval == 0 && ge) return;
		else break;
	  } else Ra = list->rate;
	}
	/* Now Ra is above Rsyn and if list != NULL, list->rate is below */
	if (list == NULL) {
	  rdivideint(&Ra, 2, &rtemp);
	  compval = rcompare(Rsyn, &rtemp);
	  if (compval < 0 || (ge && compval == 0)) *Rsyn = rtemp;
	  else *Rsyn = Ra;
	  return;
	}

	/* factor the greater rate to find the largest
	   divisor of Ra which is divisible by list->rate and
	   greater than (or equal to && ge) Rsyn
	*/
	rdivide(&Ra, &list->rate, &rtemp);
	assert(rtemp.den == 1);
	intratio = rtemp.num;
	for (divider = intratio - 1; ; divider--) {
	  assert(divider > 0);
	  if ((intratio % divider) == 0) {
		rdivideint(&Ra, divider, &rtemp);
		compval = rcompare(&rtemp, Rsyn);
		if (compval > 0 || (ge && compval == 0)) {
		  *Rsyn = rtemp;
		  return;
		}
	  }
	}
  }
}

void fpmixed(FILE *fp, rational *r) {
  unsigned int i, f;
  
  rreduce(r);
  if (r->num < 0) {
	fputc('-', fp);
	r->num = -r->num;
  }
  i = r->num / r->den;
  f = r->num % r->den;
  if (f == 0) fprintf(fp, "%d", i);
  else {
	if (i != 0) fprintf(fp, "%d ", i);
	fprintf(fp, "%d/%d", f, r->den);
  }
}

/* text is a prefixed string if not NULL
   r is the rate of the item in question
   m is a multiplier to be applied to the rate before reporting
   b is a base rate to calculate the percentage of if not NULL.
*/   
static void pctprt(FILE *fp, char *txt, rational *r,
					unsigned int m, rational *b) {
  rational ta;
  					
  fprintf(fp, "  ");
  if (txt != NULL) fprintf(fp, "%s", txt);
  rtimesint(r, m, &ta);
  fpmixed(fp, &ta);
  fprintf(fp, " bits/sec");
  if (b != NULL) {
	fprintf(fp, "  %.0lf %%",
	  ta.num * 100. * b->den / ( ta.den * b->num ) );
  }
  fputc('\n', fp);
}

/*  How to generate a TM frame:

	For each datum, i, we have a rate Ri Hz and a size Si bytes.
	Let the user designate BPMFmax. Since DBPMF is BPMF-4,
	DBPMFmax = BPMFmax-4. DBPS = sum(RiSi) (not counting synch)
	Then a lower bound for Rsyn is DBPS/DBPMFmax. Any adjustments
	must increase Rsyn.
	If Rsyn > Rmax, adjust Rsyn = Rmax * ceil(Rsyn/Rmax)
	  (i.e. round up to the nearest multiple of Rmax)
	else if Rsyn < Rmax, round up to the nearest legal rate.
*/

static void gen_frame(struct tmalloc *list) {
  rational Rmax, Rmin, DBPS, Rfrm, term;
  struct tmalloc *le;
  
  /* Calculate DBPS, Rmax and Rmin */
  DBPS = zero;
  Rmax = list->rate;
  for (le = list; le != NULL; le = le->next) {
	Rmin = le->rate;
	term.num = le->size; term.den = 1;
	rtimes(&term, &le->rate, &term);
	rplus(&term, &DBPS, &DBPS);
  }
  rdivideint(&DBPS, BPMFmax - 4, &Rsynch); /* Rsynch lower bound */
  /* Specified rate is also a lower bound */
  if ( rcompare( &mfrsort->rate, &Rsynch ) > 0 )
	Rsynch = mfrsort->rate;
  next_rate(list, &Rsynch, 1);
  if (pos_rate(list, &Rsynch, &Rmax, &Rmin, &DBPS)) {
	next_rate(list, &Rsynch, 0);
	if (pos_rate(list, &Rsynch, &Rmax, &Rmin, &DBPS))
	  compile_error(3, "Unable to generate PCM");
  }

  /* Verify proper definition of Synch and MFCtr */
  synsort->rate = Rsynch;
  mfrsort->rate = Rsynch;
  
  /* Report statistics on the frame */
  if (show(TM_STATS)) {
	int saved_response;
	
	saved_response = set_response( 1 );
    fprintf(vfile, "\n--------Frame Statistics-----------\n");
	/*  Major Frame: n minor frames, n/d Hz */
	Rfrm = (rcompare(&Rmin, &Rsynch) < 0) ? Rmin : Rsynch;
	rdivide(&Rsynch, &Rfrm, &term);
	fprintf(vfile, "Major Frame: ");
	fpmixed(vfile, &term);
	fprintf(vfile, " minor frames, ");
	fpmixed(vfile, &Rfrm);
	fprintf(vfile, " Hz\n");
	
	/* Minor Frame: n bytes, n/d Hz */
	Rfrm = (rcompare(&Rmax, &Rsynch) > 0) ? Rmax : Rsynch;
	rdivide(&Rfrm, &Rsynch, &term);
	assert(term.den == 1);
	fprintf(vfile, "Minor Frame: %d bytes, ", Ncols * term.num);
	fpmixed(vfile, &Rsynch);
	fprintf(vfile, " Hz\n");
	if (term.num != 1) {
	  fprintf(vfile, "  The minor frame is further divided into %d rows\n"
					 "  reported at ", term.num);
	  fpmixed(vfile, &Rfrm);
	  fprintf(vfile, " Hz in order to reduce overhead\n"
					 "  and report higher-rate data.\n");
	}
	fprintf(vfile, "Effective Frame Dimensions:\n"
	               "  %d Rows %d Columns\n", Nrows, Ncols);
	fprintf(vfile, "  "); fpmixed(vfile, &Rfrm); fprintf(vfile, " rows/sec\n");
	rtimesint(&Rfrm, Ncols*8, &Rfrm);
	pctprt(vfile, NULL, &Rfrm, 1, NULL);
	pctprt(vfile, "Data:  ", &DBPS, 8, &Rfrm);
	pctprt(vfile, "Synch: ", &Rsynch, 4*8, &Rfrm);
	rtimesint(&Rsynch, 4, &term); /* Synch bytes/sec */
	rplus(&term, &DBPS, &term); /* +Data bytes/sec */
	rtimesint(&term, 8, &term); /* Synch+Data bits/sec */
	rminus(&Rfrm, &term, &term); /* empty bits/sec */
	pctprt(vfile, "Empty: ", &term, 1, &Rfrm);
	if (Synchronous) {
	  if (SynchDrift == 0)
		fprintf(vfile, "Synchronous timing is exact\n");
	  else
		fprintf(vfile, "Synchronous timing will drift %ld msecs/hour\n",
						SynchDrift);
	}
	fprintf(vfile, "-------End of Frame Statistics-----\n\n");
	set_response( saved_response );
  }
}

/* copy_slots copies tmalloc slot references
   into the appropriate tmdtm entries, reporting any
   fractured data.
   also break up group cw's into individual datum cw's.
   Group collect actions go on slot of first datum.
*/   
static void copy_slots(struct tmalloc *tms) {
  struct cw *cwl;

  for (cwl = tms->sltcw; cwl != NULL; cwl = cwl->dnext)
	cwl->datum = tms->nameref;
  if (name_test(tms->nameref, NMTEST_TMDATUM)) {
	if (tms->sltcw->dnext != NULL) {
	  compile_error(1, "Datum %s is split over more than one slot",
				tms->nameref->name);
	}
  } else {
	struct grpdef *gd;
	struct nmlst *gm;
	unsigned int sw, dw, nw, sc;
	struct tmalloc *tma;
	struct cw *cwla;

	assert(name_test(tms->nameref, NMTEST_GROUP));
	gd = tms->nameref->u.grpd;
	if (tms->sltcw->dnext != NULL)
	  compile_error(1, "Group %s is split over more than one slot",
				tms->nameref->name);
	cwl = tms->sltcw;
	sc = cwl->col;
	sw = cwl->width;
	for (gm = gd->grpmems; gm != NULL; gm = gm->prev) {
	  assert(name_test(gm->names, NMTEST_TMDATUM));
	  tma = gm->names->u.tmdecl->tmdef;
	  dw = tma->size;
	  tma->sltcw = NULL;
	  while (dw > 0) {
		if (sw == 0) {
		  cwl = cwl->dnext;
		  assert(cwl != NULL);
		  sc = cwl->col;
		  sw = cwl->width;
		}
		nw = dw < sw ? dw : sw;
		tma->sltcw =
		  add_cw(tma->sltcw,
			take_slot(cwl->slot->per, cwl->slot->row, sc, nw));
		dw -= nw;
		sw -= nw;
		sc += nw;
	  }
	  if (tma->sltcw->dnext != NULL) {
		tma->flags &= ~TMDF_HOMEROW;
		compile_error(1, "Member %s is split over more than one slot",
				  gm->names->name);
	  }
	  for (cwla = tma->sltcw; cwla != NULL; cwla = cwla->dnext)
	    cwla->datum = gm->names;
	}
  }
}

void generate_pcm(void) {
  struct nm *datum;
  struct nmlst *mem;
  struct tmalloc *list, *top, *memdef;
  rational *nr, ratio;
  struct declrtor *decl;
  
  assert(global_scope != NULL);
  list = NULL;

  /* Pull Synch and MFCtr out */
  synsort = synmfr(SYNCH_NAME);
  mfrsort = synmfr(MFC_NAME);

  /* collect the groups first */
  for (datum = global_scope->names; datum != NULL; datum = datum->next) {
	if (name_test(datum, NMTEST_GROUP)) {
	  top = datum->u.grpd->tmdef;
	  for (mem = datum->u.grpd->grpmems; mem != NULL; mem = mem->prev) {
		assert(mem->names != NULL);
		assert(name_test(mem->names, NMTEST_TMDATUM));
		memdef = mem->names->u.tmdecl->tmdef;
		if (memdef == synsort || memdef == mfrsort)
		  compile_error(3, "Synch and MFCtr cannot be group members");
		if (memdef->flags & TMDF_GRPMEM)
		  compile_error(3, "Group %s member %s listed as member more than once",
			datum->name, mem->names->name);
		memdef->flags |= TMDF_GRPMEM;
		nr = &memdef->rate;
		if (mem == datum->u.grpd->grpmems) top->rate = *nr;
		else if (rcompare(nr, &top->rate) != 0)
		  compile_error(3, "Group %s member %s has different rate",
			datum->name, mem->names->name);
		decl = nr_declarator(mem->names);
		top->size += decl->size;
	  }
	  list = tmmerge(list, top);
	}
  }
  
  /* Now collect the non-group non-synch non-members */
  for (datum = global_scope->names; datum != NULL; datum = datum->next)
	if (name_test(datum, NMTEST_TMDATUM)) {
	  top = datum->u.tmdecl->tmdef;
	  if (!(top->flags & TMDF_GRPMEM)
		  && top != synsort && top != mfrsort)
		list = tmmerge(list, top);
	}

  /* Verify rate relations are legal */
  for (top = list; top != NULL && top->next != NULL; top = top->next) {
	rdivide(&top->rate, &top->next->rate, &ratio);
	if (ratio.den != 1 || ratio.num <= 0)
	  compile_error(3, "Illegal collection rate ratio:\n"
	  			   "%-12s  %d/%d Hz\n"
	  			   "%-12s  %d/%d Hz\n"
			"Ratio         %d/%d\n",
		top->nameref->name, top->rate.num, top->rate.den,
		top->next->nameref->name, top->next->rate.num, top->next->rate.den,
		ratio.num, ratio.den);
  }
  
  /* We can't go any further without any data */
  if (list == NULL) compile_error(3, "No TM data");
  
  /* Print the results (for now) */
  if (show(TM_PDEFS)) {
	fprintf(vfile, "\n-------TM Data Definitions---------\n");
    for (top = list; top != NULL; top = top->next)
	  fprintf(vfile, "%d/%d Hz %d bytes %s\n", top->rate.num,
		top->rate.den, top->size, top->nameref->name);
	fprintf(vfile, "------End of TM Data Definitions---\n\n");
  }

  gen_frame(list);
  
  /* Determine the output frame type */
  if (SynchPer == 1) {
    if (mfrsort->sltcw->col == 0) {
      TM_Data_Type = 3;
      mfrsort->flags &= ~TMDF_HOMEROW;
      synsort->flags &= ~TMDF_HOMEROW;
    } else TM_Data_Type = 1;
  } else TM_Data_Type = 2;
  
  /* If we got here, we generated a frame: */	 
  copy_slots(synsort);
  copy_slots(mfrsort);
  for (; list != NULL; list = list->next)
	copy_slots(list);
}
