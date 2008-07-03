/* pdecls.c
 * $Log$
 * Revision 1.10  2004/10/08 17:33:49  nort
 * MD5 output
 *
 * Revision 1.9  1999/08/30 17:17:49  nort
 * Added declare_convs
 *
 * Revision 1.8  1993/09/27  19:36:14  nort
 * Changes for common compiler functions, cleanup of rlog, pragmas.
 *
 * Revision 1.7  1993/05/21  19:45:55  nort
 * Added State Variable Support, eliminated indentation problem
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <unix.h>
#include "rational.h"
#include "tmcstr.h"
#include "tmc.h"
#include "nortlib.h"
#include "md5.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

static int cur_indent = 0, sw_indent = 0;

/* After adjusting indent, newlines and whitespace are swallowed until
   'real text' is seen, at which time a newline and indent is put in.
   'real text' is non-whitespace text or print_indent(NULL).
   A newline will also be triggered by print_indent(NULL) if no
   newline has been printed since the last call.
*/
adjust_indent(int di) {
  cur_indent += di;
  if (cur_indent < 0) cur_indent = 0;
  sw_indent = 1;
}

static void do_indent(void) {
  int i;
  
  fputc('\n', ofile);
  for (i = cur_indent; i > 0; i--) fputc(' ', ofile);
  sw_indent = 0;
}

/* print_indent(NULL); outputs newline and current indent: turns off sw
   print_indent(s); outputs string s, translates newlines to indent
      If (sw) leading spaces and newlines are ignored
*/
void print_indent(char *s) {
  int c;

  if (s == NULL) {
	if (sw_indent) do_indent();
	return;
  }
  for (c = *s; c != '\0'; c = *(++s)) {
	if (sw_indent) {
	  if (isspace(c)) continue;
	  else do_indent();
	}
	if (c == '\n') do_indent();
	else fputc(c, ofile);
  }
}

/* print_decl outputs declarations except for TM data in the HOMEROW */
void print_decl(struct declrtor *decl) {
  unsigned int typesout = 1;
  
  assert(decl != NULL);
  for (; decl != NULL; decl = decl->next) {
	if (!name_test(decl->nameref, NMTEST_TMDATUM)
		|| !(nr_tmalloc(decl->nameref)->flags & TMDF_HOMEROW)) {
	  if (typesout) {
		print_stat(decl->typeparts.first);
		typesout = 0;
	  } else fputc(',', ofile);
	  print_stat(decl->decl.first);
	}
  }
  if (!typesout) fputc(';', ofile);
}

void print_stat(struct statpc *spc) {
  struct nm *nr, *dy;
  struct declrtor *decl;
  char *dnm;
  struct statpc *vspc;
  struct valrec *val;
  struct tmalloc *tma;
  
  for (; spc != NULL; spc = spc->next) {
	switch (spc->type) {
	  case STATPC_CONVERT:
		assert(spc->u.cvt.cfn != NULL);
		if (spc->u.cvt.cfn->fnpre != NULL)
		  print_indent(spc->u.cvt.cfn->fnpre);
		if (spc->u.cvt.ref != NULL) print_stat(spc->u.cvt.ref);
		else putc('0', ofile);
		if (spc->u.cvt.cfn->fnpost != NULL)
		  print_indent(spc->u.cvt.cfn->fnpost);
		break;
      case STATPC_TEXT:
		if (spc->u.text == NULL) adjust_indent(0);
		else print_indent(spc->u.text);
		break;
      case STATPC_DECLS:
		print_decl(spc->u.decls);
		break;
      case STATPC_REF:
		nr = spc->u.nameref;
		assert(nr != NULL);
		assert(nr->name != NULL);
		assert(nr->type != NMTYPE_DUMMY);
		print_indent(NULL);
		if (name_test(nr, NMTEST_TMDATUM)) {
		  tma = nr_tmalloc(nr);
		  assert(tma->sltcw != NULL);
		  if ((tma->flags & TMDF_HOMEROW)
			  && tma->sltcw->home_row_text != NULL) {
			fprintf(ofile, "%s", tma->sltcw->home_row_text);
			break;
		  }
		}
		fprintf(ofile, "%s", nr->name);
		break;
      case STATPC_ADDR:
		nr = spc->u.nameref;
		decl = nr_declarator(nr);
		if (!(decl->flag & DCLF_ADDRDEF)) {
		  compile_error(1, "Undefined address datum %s", nr->name);
		  decl->flag |= DCLF_ADDRDEF;
		  decl->address = 0;
		}
		print_indent(NULL);
		fprintf(ofile, "0x%X", decl->address);
		break;
      case STATPC_DEFRUL:
		dy = spc->u.defrule.dummy;
		assert(dy->type == NMTYPE_DUMMY);
		nr = spc->u.defrule.nameref;
		assert(nr->type == NMTYPE_TMDATUM);
		dnm = dy->name;
		dy->type = NMTYPE_TMDATUM;
		dy->name = nr->name;
		dy->u.tmdecl = nr->u.tmdecl;
		print_stat(spc->u.defrule.stat.first);
		dy->type = NMTYPE_DUMMY;
		dy->name = dnm;
		break;
      case STATPC_DEPEND:
		vspc = spc->u.dep.stat.first;
		if (vspc->type == STATPC_VALCHK && vspc->u.valchk != NULL) {
		  print_indent("if (!(");
		  val = vspc->u.valchk;
		  assert(val->varname != NULL);
		  fprintf(ofile, "%s", val->varname);
		  for (val = val->next; val != NULL; val = val->next) {
			assert(val->varname != NULL);
			fprintf(ofile, "||%s", val->varname);
		  }
		  fprintf(ofile, ")) {");
		  adjust_indent(2);
		  for (val = vspc->u.valchk; val != NULL; val = val->next) {
			if (val->bitval != 0) {
			  print_indent(NULL);
			  fprintf(ofile, "%s |= 0x%X;", val->varname, val->bitval);
			  adjust_indent(0);
			}
		  }
		  print_stat(vspc->next);
		  adjust_indent(-2);
		  print_indent("}");
		  vspc = spc->u.dep.else_stat.first;
		  if (vspc != NULL) {
			adjust_indent(2);
			print_stat(vspc);
			adjust_indent(-2);
		  }
		} else {
		  print_stat(vspc);
		  if (spc->u.dep.else_stat.first != NULL) {
			compile_error(1, "depending on: else is unreachable");
			adjust_indent(0);
			print_indent("/* else clause unreachable */");
			adjust_indent(0);
		  }
		}
		break;
      case STATPC_VALID:
      case STATPC_INVALID:
		nr = spc->u.nameref;
		if (nr->type == NMTYPE_STATE)
		  print_st_valid(nr);
		else
		  print_vldtr(nr_validator(nr), spc->type == STATPC_VALID);
		break;
	  case STATPC_COMMON:
		print_stat(spc->u.cmn);
		break;
      case STATPC_TLDECLS:
      case STATPC_EXTRACT:
		compile_error(4, "Unexpected STATPC_ type %d in print_stat", spc->type);
		break;
	  default:
		compile_error(4, "Unknown STATPC_ type %d in print_stat", spc->type);
	}
  }
}

void print_decls(void) {
  struct statpc *spc;

  for (spc = program; spc != NULL; spc = spc->next) {
	assert(spc->type == STATPC_TLDECLS || spc->type == STATPC_EXTRACT
			|| spc->type == STATPC_VALID || spc->type == STATPC_INVALID);
	if (spc->type == STATPC_TLDECLS) {
	  adjust_indent(0);
	  print_stat(spc->u.stat.first);
	}
  }
}

static MD5_CTX md5ctx;
char md5_sig[16];
static md5_vpf( char *format, ... ) {
  static char *md5_buf = NULL;
  static int md5_bufsize = 80;
  va_list arg;

  if ( md5_buf == 0 ) md5_buf = new_memory( md5_bufsize );
  va_start(arg, format);
  for (;;) {
	int rv;
	rv = vsnprintf( md5_buf, md5_bufsize, format, arg );
	if ( rv < 0 || rv >= md5_bufsize-1 ) {
	  free_memory(md5_buf);
	  md5_bufsize *= 2;
	  md5_buf = new_memory(md5_bufsize);
	} else {
	  MD5Update( &md5ctx, md5_buf, rv );
	  if (show(TM_DEFS))
	    fprintf( vfile, "%s", md5_buf );
	  break;
	}
  }
  va_end(arg);
}

static void print_ddef(rational *rate, unsigned int size, char *type,
								  char *name, struct cw *cwl) {
  md5_vpf( "%d/%d Hz %d bytes %s %s", rate->num,
		  rate->den, size, type, name);
  for (; cwl != NULL; cwl = cwl->dnext) {
	md5_vpf( " (%d,%d,%d,%d)", cwl->slot->per,
		  cwl->slot->row, cwl->col, cwl->width);
    /* Don't print homerow text, since that's implementation
       dependent */
  }
  md5_vpf( "\n" );
}

static int sort_nrs( const void *a, const void *b ) {
  struct nm *nra, *nrb;
  struct tmalloc *tma_a, *tma_b;
  struct cw *cwa, *cwb;
  int rv;

  nra = *((struct nm **)a);
  nrb = *((struct nm **)b);
  tma_a = nr_tmalloc(nra);
  tma_b = nr_tmalloc(nrb);
  cwa = tma_a->sltcw;
  cwb = tma_b->sltcw;
  rv = cwb->slot->per - cwa->slot->per;
  if ( rv == 0 ) rv = cwb->slot->row - cwa->slot->row;
  if ( rv == 0 ) rv = cwb->col - cwa->col;
  if ( rv == 0 ) rv = (tma_b->flags & TMDF_GRPMEM) -
					  (tma_a->flags & TMDF_GRPMEM);
  return -rv;
}

/* PRINT RATE, SIZE, TYPE, NAME and SLOTS w/ HOMEROW */
void print_pcm(void) {
  struct nm *nr, **nrs;
  struct tmalloc *tma;
  int n_names = 0, nri = 0;
  #ifdef NONSORTEDOUTPUT
  struct nmlst *gm;
  struct grpdef *gd;
  #endif
  
  /* Count the number of nr's that apply */
  for (nr = global_scope->names; nr != NULL; nr = nr->next) {
	if (name_test(nr, NMTEST_TMDATUM) || nr->type == NMTYPE_GROUP)
	  n_names++;
  }
  nrs = new_memory( n_names * sizeof(struct nm *) );
  for (nr = global_scope->names; nr != NULL; nr = nr->next) {
	if (name_test(nr, NMTEST_TMDATUM) || nr->type == NMTYPE_GROUP)
	  nrs[nri++] = nr;
  }
  /* Now sort them by rate, row, column and group membership. */
  qsort( (void *)nrs, n_names, sizeof(struct nm *), sort_nrs );

  if (show(TM_DEFS))
	fprintf(vfile, "\n--------PCM Definition-------------\n");
  MD5Init( &md5ctx );
  for ( nri = 0; nri < n_names; nri++ ) {
	nr = nrs[nri];
	tma = nr_tmalloc(nr);
	print_ddef( &tma->rate, tma->size,
	  (nr->type == NMTYPE_GROUP) ? "Group" :
		( (tma->flags & TMDF_GRPMEM) ? "Member" : "Datum" ),
	  nr->name, tma->sltcw );
  }
  MD5Final( md5_sig, &md5ctx );
  #ifdef NONSORTEDOUTPUT
  for (nr = global_scope->names; nr != NULL; nr = nr->next) {
	if (name_test(nr, NMTEST_TMDATUM)) {
	  tma = nr_tmalloc(nr);
	  if ((tma->flags & TMDF_GRPMEM) == 0)
		print_ddef(&tma->rate, tma->size, "Datum",
					nr->name, tma->sltcw);
	} else if (nr->type == NMTYPE_GROUP) {
	  tma = nr_tmalloc(nr);
	  gd = nr->u.grpd;
	  print_ddef(&tma->rate, tma->size, "Group", nr->name, tma->sltcw);
	  for (gm = nr->u.grpd->grpmems; gm != NULL; gm = gm->prev) {
		assert(name_test(gm->names, NMTEST_TMDATUM));
		tma = nr_tmalloc(gm->names);
		print_ddef(&tma->rate, tma->size, "Member",
					gm->names->name, tma->sltcw);
	  }
	}
  }
  #endif
  if (show(TM_DEFS))
    fprintf(vfile, "-------End of PCM Definition-------\n\n");
}
