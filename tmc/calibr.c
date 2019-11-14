/* calibr.c Handles calibration information.
 * Revision 1.1  2012/06/29 17:15:52  nort
 * Initial revision
 *
 * Revision 1.5  2012/06/29 12:53:03  ntallen
 * Resolve 32-bit problem
 *
 * Revision 1.4  2009/10/02 15:50:59  ntallen
 * const char *
 *
 * Revision 1.3  2009/05/22 13:17:01  ntallen
 * Replace call to ltoa with code for binary conversion
 * Remove use of min()
 *
 * Revision 1.2  2008/07/03 18:18:48  ntallen
 * To compile under QNX6 with minor blind adaptations to changes between
 * dbr.h and tm.h
 *
 * Revision 1.1  2008/07/03 15:11:07  ntallen
 * Copied from QNX4 version V1R9
 *
 * Revision 1.24  2004/12/08 21:28:30  nort
 * Output a useful bit of information when compiling with -C
 *
 * Revision 1.23  1999/08/30 17:11:42  nort
 * Documented limitation in 1.22
 *
 * Revision 1.22  1999/06/24 17:59:53  nort
 * Change limit checks to use SHRT instead of INT for 32-bit compile.
 *
 * Revision 1.21  1998/11/20 19:02:36  nort
 * Fixed bug where input is unsigned and output is signed and
 * implicit promotion from signed short to unsigned short to
 * signed long produced problems.
 *
 * Added a note about a 32-bit problem that will require a fix.
 *
 * Revision 1.20  1998/09/16 18:55:33  nort
 * Generated function names were not specific enough:
 * %4x and %4u both got mapped to the same function!
 *
 * Revision 1.18  1997/02/10 22:08:51  nort
 * classify_conv() was being too strict:
 *   Would not treat a type as icvt if not numeric, even though
 *   an explicit conversion was provided.
 *   Forced unsigned long to be treated as double even when no
 *   conversion was required (ftype == ttype).
 *
 * Revision 1.17  1996/10/16  20:50:17  nort
 * Many changes in upgrade
 *
 * Revision 1.16  1996/08/08  18:22:39  nort
 * Compiled, not tested.
 *
 * Revision 1.15  1996/07/30  19:07:29  nort
 * Bug causing some calibrations to fail
 *
 * Revision 1.14  1996/03/27  02:44:36  nort
 * Fixed a SIGSEGV and an assert failure under error conditions.
 * Now supports narrow format for char conversions
 * Shortened long warning for narrow formats
 *
 * Revision 1.13  1995/11/15  04:20:02  nort
 * Added support for explicit conversion functions
 *
 * Revision 1.12  1995/10/18  02:03:23  nort
 * *** empty log message ***
 *
 * Revision 1.11  1994/02/17  01:31:52  nort
 * Possible kluge to fix erroneous generated functions:
 * allowed for wrapping of (nX+r) modulo 65536 which will only
 * work correctly if n,X and r are all shorts, so force r in
 * particular to be short.
 *
 * Revision 1.10  1993/09/27  19:39:07  nort
 * Changes to support common compiler functions, cleanup.
 * Tuning of calibration conversions.
 *
 * Revision 1.9  1993/04/01  22:04:38  nort
 * Restructuring & cleaned up output
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include <ctype.h>
#include <stdbool.h>
#include "nl.h"
#include "rational.h"
#include "tmcstr.h"
#include "tmc.h"
#include "calibr.h"

#ifdef __DOCUMENTATION

  The main external interfaces:

    void get_cfnc(struct sttmnt *s, int cflg) {
      Called from tmc.y when a conversion is parsed by either
      convert(), text() or display() syntax.
    }
    
    struct cvtfunc *mk_cvt_func( char *name, char syntax ) {
      Called from tmc.y during TM typedef when an explicit 
      conversion function is specified.
    }

    void add_pair(struct pairlist *list, double v0, double v1) {
      Called from tmc.y when parsing calibrations.
      Sorts the values as they are input.
    }

    void add_calibration(struct nm *type0, struct nm *type1,
                          struct pairlist *pl) {
      Called from tmc.y when a calibration has been parsed
    }
    
    void declare_convs(void) {
      Called from tmcmain.c/main() to decide which conversions 
      need to be generated.
    }
    
    classify_conv()
    specify_conv()

  Internal Functions:

    static double cal_convert(double iv, struct calibration *cal) {
        Uses the specified calibration to produce an output based on the
        input. Produces warnings if extrapolation is necessary.
    }

    static void txtfmt(char *buf, char *format, struct pfmt *pformat,
                double ov, unsigned int type) {
        Given format and value, produces output text. Used for
        8-bit text conversions.
    }

    struct intcnv *find_ndr(struct calibration *cal,
          int32_t x0, int32_t x1, double m, double b) {
       Given slope and intercept and input range, generates a chain
       of regions over which a simple linear integer expression will
       produce the predicted results.
       New form: y = (nx+r)/d + y0
    }

    static void int_conv(struct calibration *cal,
                         double *input_min, double *input_max,
                         double yscale, struct intcnvl *cl) {
      generates chain of regions where simple linear conversion
      is possible based on calibration. Sets *input_min and
      _max to output min and max.
      calls:
        find_ndr
    }

    static void gen_itc_code(int n, struct intcnv *p, char *ovtxt) {
      generates integer-integer conversion code for the n regions
      pointed to by p. ovtxt holds the name of the variable into
      which the final result is to be placed.
      recurses.
    }
    
    static void gen_dtc_code();
    
    static void gen_int_icvt();
    static void gen_int_cvt();
    static void gen_doub_cvt();
    static void gen_doub_icvt();
    static void gen_int_tcvt();
    static void gen_e_tcvt();
    static void generate_tfunc();
    identify_calibrations();
    generate_calibrations();

#endif

static struct calibration *calibrations;

/* add_pair sorts the values as they are input */
void add_pair(struct pairlist *list, double v0, double v1) {
  struct pair *np, *p0, *p1;
  
  if (list->pairs == NULL) list->npts = 0;
  np = new_memory(sizeof(struct pair));
  np->next = NULL;
  np->v[0] = v0;
  np->v[1] = v1;
  p0 = NULL;
  p1 = list->pairs;
  while (p1 != NULL) {
    if (v0 < p1->v[0]) break;
    p0 = p1;
    p1 = p1->next;
  }
  np->next = p1;
  if (p0 == NULL) list->pairs = np;
  else p0->next = np;
  list->npts++;
}

void add_calibration(struct nm *type0, struct nm *type1, struct pairlist *pl) {
  struct calibration *nc;
  int i;
  struct pair *p;

  assert(type0 != NULL && type1 != NULL);
  for (nc = calibrations; nc != NULL; nc = nc->next) {
    for (i = 0; i < 2; i++) {
      if (type0 == nc->type[i] && type1 == nc->type[i^1])
        compile_error(2, "Illegal recalibration of %s and %s",
              type0->name, type1->name);
    }
  }
  nc = new_memory(sizeof(struct calibration));
  nc->type[0] = type0;
  nc->type[1] = type1;
  nc->pl = *pl;
  nc->next = calibrations;
  calibrations = nc;
  nc->flag = CALB_XUNIQ;
  if (pl->npts < 2) compile_error(1, "Poor excuse for a calibration");
  else for (p = pl->pairs; p->next != NULL; p = p->next) {
    if (p->v[0] == p->next->v[0]) {
      nc->flag &= ~CALB_XUNIQ;
      break;
    }
  }
}

struct pfmt {
  unsigned char flags;
  unsigned char width;
  unsigned char prec;
  unsigned char code;
  unsigned char twidth;
  unsigned char cpos;
};
#define PF_MINUS 1
#define PF_ZERO 2
#define PF_PREC 4
#define PF_LONG 8
#define PF_CUSTOM 0x10
#define PF_CONVERSION 0x20
/*
parse_txtfmt

parse_txtfmt(char *iformat, char pformat[6],
             char *pretext, char *format, char *posttext);

parse_txtfmt pulls apart a c-style conversion format expression to
determine the width, precision, justification and conversion mode.  This
information is laid out in pformat.
*/
static void parse_txtfmt(char *iformat, struct pfmt *pformat,
             char *pretext, char *format, char *posttext) {
  char *s, *f;

  pformat->flags = pformat->twidth = 0;
  for (s = iformat; *s != '\0' && *s != '%'; s++) {
    if (*s == '\\' && s[1] != '\0') s++;
    if (pretext != NULL) *pretext++ = *s;
    pformat->twidth++;
  }
  if (pretext != NULL) *pretext = '\0';
  if (*s == '\0') return;
  f = s;
  pformat->flags |= PF_CONVERSION;
  pformat->cpos = pformat->twidth;
  for (;;) {
    if (*(++s) == '-') {
      pformat->flags |= PF_MINUS;
      s++;
    } else if (*s == '+' || *s == ' ' || *s == '#') s++;
    else break;
  }
  if (*s == '0') {
    pformat->flags |= PF_ZERO;
    s++;
  }
  pformat->width = 0;
  while (isdigit(*s)) pformat->width = pformat->width*10 + *s++ - '0';
  pformat->twidth += pformat->width;
  if (*s == '.' && isdigit(*(++s))) {
    pformat->prec = 0;
    pformat->flags |= PF_PREC;
    while (isdigit(*s)) pformat->prec = pformat->prec*10 + *s++ - '0';
  }
  if (*s == 'l') {
    pformat->flags |= PF_LONG;
    s++;
  }
  pformat->code = *s++;
  if (format != NULL) {
    while (f != s) *format++ = *f++;
    *format = '\0';
  }
  if (posttext != NULL) {
    while (*s != '\0') {
      if (*s == '\\' && s[1] != '\0') s++;
      *posttext++ = *s++;
      pformat->twidth++;
    }
  }
}

static int extrap_required = 0;

static double cal_interp(double iv, struct pair *p) {
  double v0, v0a, v1, dv0, dv1;
  
  assert(p != NULL && p->next != NULL);
  v0 = p->v[0];
  v0a = p->next->v[0];
  if (v0 == v0a)
    compile_error(3, "Ill-specified conversion in cal_interp");
  if (iv < v0 || iv > v0a) extrap_required = 1;
  dv0 = v0a - v0;
  v1 = p->v[1];
  dv1 = p->next->v[1] - v1;
  return(v1 + dv1 * (iv - v0)/dv0);
}

/* Uses the specified calibration to produce an output based on the
   input. Produces warnings if extrapolation is necessary.
*/
static double cal_convert(double iv, struct calibration *cal) {
  struct pair *p;
  
  if (cal == NULL) return(iv);
  p = cal->pl.pairs;
  assert(p != NULL);
  if (iv == p->v[0] || p->next == NULL) return(p->v[1]);
  if (iv < p->v[0]) return(cal_interp(iv, p));
  for (;;) {
    if (iv < p->next->v[0]) return(cal_interp(iv, p));
    else if (iv == p->next->v[0]) return(p->next->v[1]);
    else if (p->next->next == NULL) return(cal_interp(iv, p));
    else p = p->next;
  }
}

/* Given format and value, produces output text. Used for
   8-bit text conversions. Returns non-zero on error
*/
static int txtfmt(char *buf, char *format, struct pfmt *pformat,
            double ov, unsigned int type) {
  double mn, mx;
  union {
    int8_t c;
    uint8_t uc;
    int i;
    uint16_t ui;
    int32_t l;
    uint32_t ul;
  } u;
  // char lbuf[80], c;
  
  if (tolower(pformat->code) == 'f' || TYPE_FLOATING(type)) {
    if (tolower(pformat->code == 'f' &&  fabs(ov) > 1e30)) {
      compile_error(2, "Text overflow during conversion");
      return(1);
    }
    sprintf(buf, format, ov);
  } else if (TYPE_INTEGRAL(type)) {
    switch (type & (INTTYPE_CHAR | INTTYPE_LONG | INTTYPE_UNSIGNED)) {
      case 0: mn = INT16_MIN; mx = INT16_MAX; break;
      case INTTYPE_UNSIGNED: mn = 0; mx = UINT16_MAX; break;
      case INTTYPE_CHAR: mn = INT8_MIN; mx = INT8_MAX; break;
      case INTTYPE_UNSIGNED | INTTYPE_CHAR: mn = 0; mx = UINT8_MAX; break;
      case INTTYPE_LONG: mn = INT32_MIN; mx = INT32_MAX; break;
      case INTTYPE_UNSIGNED | INTTYPE_LONG: mn = 0; mx = UINT32_MAX; break;
      default:
        compile_error(2, "Internal: Strange type %X in txtfmt", type);
        return(1);
    }
    if (ov < mn || ov > mx) {
      compile_error(2, "Conversion out of range");
      return(1);
    }
    if (tolower(pformat->code) == 'b') {
      char zchar = pformat->flags & PF_ZERO ? '0' : ' ';
      unsigned char mask = 1 << (pformat->width - 1);
      int n = pformat->width;
      int i = 0;
      while ( n > 8 ) {
        buf[i++] = zchar;
        --n;
      }
      u.uc = ov;
      while ( mask ) {
        if ( mask == 1 ) zchar = '0';
        if ( mask & u.uc ) {
          buf[i] = '1';
          zchar = '0';
        } else buf[i] = zchar;
        mask >>= 1;
        ++i;
      }
      buf[i] = '\0';
      // u.l = ov;
      // ltoa(u.l, lbuf, 2);
      // u.i = pformat->width - strlen(lbuf);
      // if (u.i < 0) u.i = 0;
      // strcpy(buf+u.i, lbuf);
      // c = pformat->flags & PF_ZERO ? '0' : ' ';
      // while (u.i > 0) buf[--u.i] = c;
    } else switch (type & (INTTYPE_CHAR | INTTYPE_LONG | INTTYPE_UNSIGNED)) {
      case 0: compile_error(2, "Invalid unqualified int"); u.i = ov; sprintf(buf, format, u.i); break;
      case INTTYPE_UNSIGNED: u.ui = ov; sprintf(buf, format, u.ui); break;
      case INTTYPE_CHAR: u.c = ov; sprintf(buf, format, u.c); break;
      case INTTYPE_UNSIGNED | INTTYPE_CHAR:
        u.uc = ov; sprintf(buf, format, u.uc); break;
      case INTTYPE_LONG: u.l = ov; sprintf(buf, format, u.l); break;
      case INTTYPE_UNSIGNED | INTTYPE_LONG:
        u.ul = ov; sprintf(buf, format, u.ul); break;
    }
  } else {
    compile_error(2, "Illegal text output type %X", type);
    return(1);
  }
  return(0);
}

static int cvtno = 0;
#define CVT_PREFIX "_CVT_"
#define COL_WIDTH 72

/* pre is either "(" or "[" */
static void gen_cvt_name(struct cvtfunc *cvs, char pre) {
  char format[20];
  
  sprintf(format, CVT_PREFIX "%d%c", cvtno++, pre);
  cvs->fnpre = strdup(format);
  cvs->fnpost = (pre == '(') ? ")" : "]";
}

/* 8-bit text conversions */
static void generate_text_array( struct tmtype *ftype ) {
  struct caldef *cdf;
  struct cvtfunc *cvs;
  struct nm *fname, *tname;
  struct tmtype *ttype;
  struct calibration *cal;
  char format[20], buf[80];
  struct pfmt pformat;
  int i, col, len, j, too_wide = 0;
  double ov;

  assert( ftype != 0 );
  cdf = &ftype->caldefs;
  cvs = cdf->tcvt; assert( cvs != 0 );
  fname = ftype->decl->nameref;
  tname = ftype->convert;
  if ( tname != 0 ) {
    assert( tname->type == NMTYPE_TMTYPE );
    ttype = tname->u.tmtdecl;
  } else {
    ttype = ftype;
    tname = fname;
  }
  cal = cdf->cal;

  gen_cvt_name(cvs, '[');
  parse_txtfmt(ftype->txtfmt, &pformat, NULL, format, NULL);
  print_indent(NULL);
  fprintf(ofile, "/* Text array for %s -> %s */\n",
    fname->name, tname->name );
  fprintf(ofile, "static char %s256][%d] = {\n ",
    cvs->fnpre, pformat.width+1);
  col = 1;
  extrap_required = 0;

  for (i = 0; i < 256; i++) {
    ov = i;
    /* Adjust for sign */
    if (i >= 128 && !(ftype->decl->type & INTTYPE_UNSIGNED))
      ov -= 256;
    /* Adjust for size */
    for (len = ftype->decl->size-1; len > 0; len--) ov *= 256;
    /* Convert if necessary */
    ov = cal_convert(ov, cal);
    if (txtfmt(buf, format, &pformat, ov, ttype->decl->type)) {
      compile_error(3, "Error converting %s to %s text",
        fname->name, tname->name );
    }
    if (i > 0) { fputc(',', ofile); col++; }
    /* verify that output text is of the specified width. */
    len = strlen(buf);
    if ( len < pformat.width) {
      compile_error(2, "Error converting %s to %s text",
        fname->name, tname->name );
      compile_error(2, "Conversion length error: i=%d, ov=%g", i, ov);
    } else if ( len > pformat.width ) {
      for ( j = 0; j < pformat.width; j++ ) buf[j] = '*';
      buf[ pformat.width ] = '\0';
      too_wide = 1;
      len = pformat.width;
    }
    if (col + len + 3 >= COL_WIDTH) { fprintf(ofile, "\n "); col = 1; }
    fprintf(ofile, " \"%s\"", buf);
    col += len+3;
  }
  fprintf(ofile, "\n};\n");
  if (too_wide)
    compile_error(1,
      "Format \"%s\" may be narrow for input type %s",
      ftype->txtfmt, fname->name );
  if (extrap_required)
    compile_error(0, "Extrapolation required converting %s to %s",
      fname->name, tname->name );
}

/* determine minimum and maximum output values based on format */
static void format_range(struct pfmt *pformat, double *fmt_min,
        double *fmt_max, double *yscale) {
  *yscale = 1.0;
  switch (tolower(pformat->code)) {
    case 'b':
      *fmt_max = pow(2.0, pformat->width) - 1.;
      *fmt_min = 0.;
      break;
    case 'c':
      *fmt_max = UINT16_MAX;
      *fmt_min = 0;
      break;
    case 'f':
      if (pformat->width < pformat->prec+2)
        compile_error(3, "I can't deal with format %%%d.%d%c",
                pformat->width, pformat->prec, pformat->code);
      *fmt_max = pow(10.0, pformat->width-1) - 1;
      *fmt_min = - (pow(10.0, (pformat->width-2)) - 1);
      *yscale = pow(10.0, pformat->prec);
      break;
    case 'd':
      *fmt_max = pow(10.0, pformat->width) - 1;
      *fmt_min = - (pow(10.0, (pformat->width-1)) - 1);
      break;
    case 'o':
      *fmt_max = pow(8.0, pformat->width) - 1.;
      *fmt_min = 0.;
      break;
    case 'u':
      *fmt_max = pow(10.0, pformat->width) - 1.;
      *fmt_min = 0.;
      break;
    case 'x':
      *fmt_max = pow(16.0, pformat->width) - 1.;
      *fmt_min = 0.;
      break;
    default: /* cannot determine range */
      *fmt_min = 0;
      *fmt_max = -1;
      break;
  }
}

static void type_range( unsigned int type, double *min, double *max) {
  if(TYPE_INTEGRAL(type)) {
    if (type & INTTYPE_CHAR) {
      if (type & INTTYPE_UNSIGNED) { *min = 0; *max = UINT8_MAX; }
      else { *min = INT8_MIN; *max = INT8_MAX; }
    } else if (type & INTTYPE_LONG) {
      if (type & INTTYPE_UNSIGNED) { *min = 0; *max = UINT32_MAX; }
      else { *min = INT32_MIN; *max = INT32_MAX; }
    } else if (type & INTTYPE_UNSIGNED) { *min = 0; *max = UINT16_MAX; }
    else { *min = INT16_MIN; *max = INT16_MAX; }
  } else { *min = 0; *max = -1; }
}

struct intcnv {
  struct intcnv *next;
  int32_t x0, x1;
  int32_t n, r, d, y0;
  int flag;
};
#define ICNV_INT 1

struct intcnvl {
  struct intcnv *first, *last;
  int n_regions;
};

/* Given slope and intercept and input range, generates a chain
   of regions over which a simple linear integer expression will
   produce the predicted results.
   New form: y = (nx+r)/d + y0
*/
static struct intcnv *find_ndr(struct calibration *cal,
        int32_t x0, int32_t x1, double m, double b) {
  double drbest, dr, ddx;
  int sign_m;
  int32_t dmax, nbest, n, dbest, d, r, rmin, rmax;
  int32_t x, y, dx, dtx, dx1, ty;
  int32_t y0, dy;
  int32_t op_range, dlast;
  struct intcnv *result, *ic, *ica;

  if (show(CONVERSIONS))
    fprintf(vfile, "(%s => %s)\n"
                   "Looking for rational expression for:\n"
                   "  Y = %gX%+g  where %d <= X <= %d\n",
                   cal->type[0]->name, cal->type[1]->name,
                   m, b, x0, x1);

  /* Make m positive */
  if (m < 0.0) {
    m = -m;
    b = -b;
    sign_m = -1;
  } else sign_m = 1;
  b += .5;

  assert(x1 >= x0);
  op_range = INT16_MAX;
  dlast = 1;
  result = NULL;
  for (;;) {
    ddx = (double) x1 - x0;
    y0 = floor(m*x0+b);
    dy = 0;
    y = floor(m*x1+b);
    if (y < y0) {
      dy = y0 - y;
      y0 = y;
    } else dy = y - y0;

    for (;;) {
      if (dy == 0) {
        nbest = 0;
        drbest = 0;
        dbest = 1;
        break;
      } else {
        dmax = op_range/dy;
        if (dmax > UINT16_MAX) dmax = UINT16_MAX; /* arbitrary limit */
      }

      drbest = -1;
      for (dlast = 1; dlast <= dmax; dlast++) {
        n = floor(m*dlast + .5);
        dr = fabs((m - ((double)n)/dlast) * ddx);
        if (drbest < 0 || dr < drbest) {
          drbest = dr;
          nbest = n;
          dbest = dlast;
          if (dr == 0.) break;
        }
      }
      if (op_range == INT16_MAX && drbest > 1.0) op_range = INT32_MAX;
      else break;
    }
    if (drbest >= 2.0)
      /* compile_error(3, "Unable to derive ratio: drbest = %.1lf", drbest); */
      compile_error(3, "(%s => %s)\n"
                   "Unable to derive rational expression for:\n"
                   "  Y = %gX%+g  where %d <= X <= %d\n"
                   "  drbest = %.1lf",
                   cal->type[0]->name, cal->type[1]->name,
                   m, b, x0, x1, drbest);
    /* Now see how far this gets us */
    n = nbest; d = dbest;
    y = floor(m*x0+b);
    rmin = (y-y0)*d;
    rmax = rmin + d - 1;
    if (show(CONVERSIONS))
      fprintf(vfile, "Checking values for %d/%d\n", n, d);
    if (ddx > UINT16_MAX) dtx = ddx/UINT16_MAX;
    else dtx = 1;
    if (INT32_MAX-dtx < x1) dx1 = INT32_MAX-dtx;
    else dx1 = x1;
    for (x = x0 + dtx; x <= dx1; x += dtx) {
      y = floor(m*x+b);
      r = (y - y0)*d - (x - x0)*n;
      if (r < rmin) {
        r += d - 1;
        if (r < rmin) break;
        else if (r < rmax) rmax = r;
      } else if (r > rmax) break;
      else if (r > rmin) rmin = r;
    }
    if (x > dx1) x = x1;
    else x -= dtx; /* Last value which worked, x1 if all worked */
    if (x < x1 && op_range == INT16_MAX) {
      op_range = INT32_MAX;
      continue;
    }
    /* 32-bit issue: Here we redefine r assuming n*x is OK */
    r = rmin - n*x0;
    if (y0 > 0 && (y0+dy) * (double)d <= op_range) {
      r += y0 * d;
      y0 = 0;
    }
    if (r < 0 && r+rmax-rmin >= 0) {
      rmin -= r;
      r = 0;
    }
    y0 *= sign_m;
    d *= sign_m;
    if (d < 0) { d *= -1; n *= -1; r *= -1; op_range = INT32_MAX; }
    /* This following test is to account for a bug where the
       source type is unsigned short but the result is signed
       and goes negative. Since signed ints are promoted to
       unsigned, if the result is assigned to a long, you'll
       get the wrong answer. At this point, we don't know
       what the source type is, so we'll just punt and promote
       to long anyway.
    */
    if ( op_range == INT16_MAX && y0 < 0 ) op_range = INT32_MAX;
    ic = new_memory(sizeof(struct intcnv));
    ic->next = NULL;
    ic->x0 = x0;
    ic->x1 = x;
    ic->n  = n;
    ic->d  = d;
    ic->r  = r;
    ic->y0 = y0;
    ic->flag = (op_range == INT16_MAX) ? ICNV_INT : 0;
    if (result == NULL) result = ica = ic;
    else {
      ica->next = ic;
      ica = ic;
    }
    if (show(CONVERSIONS))
      fprintf(vfile,
        "%d - %d : (%d*X + (%d (+%d))) / %d + (%d) [%s]\n",
          x0, x, n, r, rmax-rmin, d, y0, op_range==INT16_MAX ?
          "short" : "long");
    /* dtx was set for rmin/rmax determination */
    if (drbest < 2.0) { /* change this to != 0.0 */
      if (show(CONVERSIONS)) {
        fprintf(vfile, "Double Checking: ");
        fflush(vfile);
      }
      if (INT32_MAX-dtx < x) dx1 = INT32_MAX-dtx;
      else dx1 = x;
      for (dx = x0; dx <= dx1; dx += dtx) {
        y = sign_m * floor(m*dx+b);
        ty = (n*dx + r)/d + y0;
        if (ty != y)
          compile_error(3,
            "Conversion(%s => %s): f(%d) = %d but I got %d\n",
            cal->type[0]->name, cal->type[1]->name, dx, y, ty);
      }
      if (show(CONVERSIONS)) fprintf(vfile, "passed\n");
    }
    if (x == x1) break;
    x0 = x+1;
  }
  return(result);
}

/* generate chain of regions where simple linear conversion
   is possible based on calibration. Sets *input_min and
   *input_max to the output min and max.
*/
static void int_conv(struct calibration *cal,
                     double *input_min, double *input_max,
                     double yscale, struct intcnvl *cl) {
  struct pair *p;
  double m, b, fx, y, cvt_min, cvt_max;
  int32_t x0, x1, x;
  struct intcnv *cv;
  
  if ( show(CONVERSIONS) )
    fprintf( vfile, "int_conv( %s -> %s )\n",
      cal->type[0]->name, cal->type[1]->name );
  if (*input_max > INT32_MAX)
    compile_error(3, "Cannot convert uint32_t (%s -> %s)",
      cal->type[0]->name, cal->type[1]->name );
  else if (*input_max > UINT16_MAX)
    compile_error(1, "Conversion (%s => %s) of int32_t cannot be fully validated",
      cal->type[0]->name, cal->type[1]->name);
  x0 = *input_min;
  x1 = *input_max;
  cvt_min = cvt_max = 0.;
  assert(cal->flag & CALB_XUNIQ);
  p = cal->pl.pairs;
  assert(p != NULL);
  if (p->next == NULL) {
    cvt_min = cvt_max = p->v[1];
    cl->first = cl->last = find_ndr(cal, x0, x1, 0., p->v[1]);
    cl->n_regions = 1;
    while (cl->last->next != NULL) {
      cl->last = cl->last->next;
      cl->n_regions++;
    }
  } else {
    cl->first = cl->last = NULL;
    cl->n_regions = 0;
    for (;;) {
      while (p->next->next != NULL && x0 > p->next->v[0]) p = p->next;
      if (p->next->next == NULL) x = x1;
      else x = p->next->v[0] < x1 ? p->next->v[0] : x1;
        /* min(p->next->v[0], x1); */
      m = yscale * (p->next->v[1] - p->v[1])/(p->next->v[0] - p->v[0]);
      b = yscale * p->v[1] - m * p->v[0];
      for (fx = x0; ; fx = x) {
        y = m*fx + b;
        if (y < cvt_min) cvt_min = y;
        if (y > cvt_max) cvt_max = y;
        if (fx == x) break;
      }
      cv = find_ndr(cal, x0, x, m, b);
      if (cl->last == NULL) cl->first = cl->last = cv;
      else { cl->last->next = cv; cl->last = cv; }
      cl->n_regions++;
      while (cl->last->next != NULL) {
        cl->last = cl->last->next;
        cl->n_regions++;
      }
      assert(cl->last->x1 == x);
      if (x == x1) break;
      x0 = x+1;
    }
  }
  *input_min = cvt_min;
  *input_max = cvt_max;
}

/* generates integer-integer conversion code for the n regions
   pointed to by p. ovtxt holds the name of the variable into
   which the final result is to be placed.
*/
static void gen_itc_code(int n, struct intcnv *p, char *ovtxt) {
  int n1, i;
  struct intcnv *p1;

  adjust_indent(2);
  if (n == 1) {
    print_indent(NULL);
    fprintf(ofile, "%s = ", ovtxt);
    if (p->n == 0) {
      p->y0 += p->r/p->d;
      fprintf(ofile, "%d", p->y0);
    } else {
      if (p->d == 1) {
        p->r += p->y0;
        p->y0 = 0;
      }
      if (p->r != 0) print_indent("(");
      print_indent("x");
      if ((p->flag & ICNV_INT) == 0)
        fprintf(ofile, "*(%dL)", p->n);
      else if (p->n != 1) fprintf(ofile, "*(%d)", p->n);
      if (p->r != 0) {
        /* This portion of code will generate invalid output in
           a 32-bit environment. The solution is to either add
           a flag to TMC to indicate 32-bit output, or to
           sanitize the code to make it generally safe. The
           latter is probably more desirable, since it will
           avoid the tricks of allowing wrapping when it's OK.
           The format would then be y = (n*(x-x0)+r)/d + y0.
           Currently y = (x*(n)+r)/d + y0
           This might require changes to the calculation of r
           Mitigated 2012-06-29
        */
        #if UINT_MAX > 65536L
          fprintf(ofile, "%+d)", p->r);
        #else
          if (p->flag & ICNV_INT) fprintf(ofile, "%+d)", (int16_t) p->r);
          else fprintf(ofile, "%+d)", p->r);
        #endif
      }
      if (p->d != 1) fprintf(ofile, "/(%d)", p->d);
      if (p->y0 != 0) fprintf(ofile, "%+d", p->y0);
    }
    print_indent(";");
    adjust_indent(0);
  } else {
    n1 = n/2;
    for (p1 = p, i = n1; i > 0; i--) {
      assert(p->next != NULL);
      p1 = p1->next;
    }
    print_indent(NULL);
    fprintf(ofile, "if (x < %d) {", p1->x0);
    gen_itc_code(n1, p, ovtxt);
    print_indent("} else {");
    gen_itc_code(n-n1, p1, ovtxt);
    print_indent("}");
  }
  adjust_indent(-2);
}

/* converts x to ov using cal. If ttype is integral adds floor(x+.5) */
/*
  Take an input range as an argument against which to test
  Calculate output values as derived from the input args
  Output range calculation could be accomplished directly in a 
  separate function with few calculations; given input range, for 
  all x cal points within the range, test y against current 
  range. If the input range exceeds the x cal points, do 
  extrapolation above and below. If input range is unknown (e.g. 
  double) use all Y points and if outside ranges have non-zero 
  slopes, specify range as unknown.
*/
static void gen_dtc_code(struct pair *p, int npts,
                        int from_int, int to_int) {
  int apts, i;
  struct pair *p1;
  double slope, intcpt;
  
  assert(npts >= 2);
  adjust_indent(2);
  if (npts > 2) {
    apts = (npts+1)/2;
    for (i = apts-2, p1 = p->next; i > 0; i--, p1 = p1->next);
    print_indent(NULL);
    fprintf(ofile, "if (x <= ");
    if (from_int) fprintf(ofile, "%d", (int32_t)p1->v[0]);
    else fprintf(ofile, "%.8g", p1->v[0]);
    fprintf(ofile, ") {");
    gen_dtc_code(p, apts, from_int, to_int);
    print_indent(NULL);
    fprintf(ofile, "} else {");
    gen_dtc_code(p1, npts-apts+1, from_int, to_int);
    print_indent(NULL);
    fprintf(ofile, "}");
  } else {
    print_indent(NULL);
    fprintf(ofile, "ov = ");
    slope = (p->next->v[1] - p->v[1])/(p->next->v[0] - p->v[0]);
    intcpt = p->v[1] - slope * p->v[0];
    if (to_int) intcpt += 0.5;
    if ( slope != 0 ) {
      if (to_int) fprintf(ofile, "floor(");
      if ( slope != 1.0 ) fprintf( ofile, "%.8g * ", slope );
      fprintf( ofile, "x" );
      if (intcpt != 0.) fprintf(ofile, "%+.8g", intcpt);
      if (to_int) putc(')', ofile);
    } else {
      if ( to_int )
        fprintf( ofile, "%.0lf", floor( intcpt ) );
      else
        fprintf( ofile, "%.8g", intcpt );
    }
    putc(';', ofile);
  }
  adjust_indent(-2);
}

/* We know a conversion exists */
static void gen_doub_cvt( struct tmtype *ftype ) {
  struct caldef *cdf;
  struct cvtfunc *cvs;
  struct nm *tname;
  struct tmtype *ttype;
  struct calibration *cal;
  double in_min, in_max;

  assert( ftype != 0 );
  cdf = &ftype->caldefs;
  cvs = cdf->cvt;          assert( cvs != 0 );
  cal = cdf->cal;
  tname = ftype->convert;
  type_range( ftype->decl->type, &in_min, &in_max );
  cvs->out_min = in_min;
  cvs->out_max = in_max;

  if ( tname == 0 || cal == 0 ) return;
  /* cal == 0 is actually an error, but will already have been 
    reported by identify_calibrations()
  */
  assert( tname->type == NMTYPE_TMTYPE );
  ttype = tname->u.tmtdecl;

  /* Print function entry code */
  gen_cvt_name(cvs, '(');
  print_indent(NULL);
  fprintf(ofile, "/* doub_cvt for %s -> %s */\n",
          ftype->decl->nameref->name, tname->name );
  fprintf(ofile, "double %s %s x ) {", cvs->fnpre,
                    ftype->decl->nameref->name);
  fprintf( ofile, "\n  double ov;\n" );

  /* converts x to ov using cal.  */
  gen_dtc_code( cal->pl.pairs, cal->pl.npts,
                TYPE_INTEGRAL(ftype->decl->type), 0 );
  fprintf(ofile, "\n  return(ov);\n}");
  
  { double out_min, out_max;
    struct pair *p;
    int in_uk;
    double v;

    out_max = -1; out_min = 0;
    p = cal->pl.pairs;
    in_uk = in_min > in_max;

    /* Define initial value. If in_uk, we'll check extrema later */
    if (! in_uk) {
      out_min = out_max = cal_convert( in_min, cal );
      v = cal_convert( in_max, cal );
      if ( v < out_min ) out_min = v;
      else if ( v > out_max ) out_max = v;
    }
    if ( p->next != 0 ) {
      if ( ! in_uk || p->v[1] == p->next->v[1] ) {
        for ( ; p != 0; p = p->next ) {
          if ( in_uk ||
              ( p->v[0] >= in_min && p->v[0] <= in_max ) ) {
            if ( p->v[1] < out_min ) out_min = p->v[1];
            else if ( p->v[1] > out_max ) out_max = p->v[1];
          }
          /* Here we check the final extrema if in_uk */
          if ( in_uk && p->next != 0 && p->next->next == 0 &&
                p->v[1] != p->next->v[1] ) {
            out_min = 0; out_max = -1; break;
          }
        }
      }
    }
    cvs->out_min = out_min;
    cvs->out_max = out_max;
  }
}

/* selects a type based on the specified range and desired output 
   type. If the specified cvt_min > cvt_max, the range is 
   unknown or unrestricted. Will use the ttype if it is integral, 
   but will compare it against the range and issue a warning if 
   it is insufficient. Otherwise, if a suitable integer type can 
   be selected, it will be.
*/
static char *pick_a_type( struct tmtype *ftype, struct tmtype *ttype,
              double cvt_min, double cvt_max, unsigned int *tptr ) {
  char *ret_type = NULL;
  unsigned int tcode;

  if ( ttype != 0 && TYPE_INTEGRAL( ttype->decl->type ) ) {
    double type_min, type_max;
    
    ret_type = ttype->decl->nameref->name;
    tcode = ttype->decl->type;
    type_range( tcode, &type_min, &type_max );
    if ( ftype != 0 && ( cvt_min > cvt_max || cvt_max > type_max ||
         cvt_min < type_min ) )
      compile_error( 1,
        "Conversion for type %s may overflow output type %s",
        ftype->decl->nameref->name, ret_type );
  } else if ( cvt_min <= cvt_max ) {
    if ( cvt_min >= 0 ) {
      if ( cvt_max <= UINT16_MAX ) {
        ret_type = "uint16_t";
        tcode = 0x1A;
      } else if ( cvt_max <= UINT32_MAX ) {
        ret_type = "uint32_t";
        tcode = 0x16;
      }
    } else if ( cvt_min >= INT32_MIN ) {
      if ( cvt_min >= INT16_MIN && cvt_max <= INT16_MAX ) {
        ret_type = "int16_t";
        tcode = 0x0A;
      } else if ( cvt_max <= INT32_MAX ) {
        ret_type = "int32_t";
        tcode = 0x06;
      }
    }
  }
  if ( ret_type == 0 ) {
    tcode = INTTYPE_DOUBLE;
    ret_type = "double";
  }
  if ( tptr != 0 ) *tptr = tcode;
  return ret_type;
}

static const char *non_null( const char *s ) {
  return s == 0 ? "" : s;
}

/* gen_doub_icvt() generates an icvt function for a type 
   requiring a double conversion (e.g. source type is double).
   Uses the cvt function, but scales it appropriately.

   I will decide the return type in the same manner as in 
   gen_int_icvt(): If ttype is integral, I will use that. 
   Otherwise, I will use the output range of the conversion to 
   select an appropriate type. If the output range is unknown or 
   too wide, the output will be double.
*/
static void gen_doub_icvt( struct tmtype *ftype ) {
  struct caldef *cdf;
  struct nm *tname;
  struct tmtype *ttype;
  double yscale;
  char *ret_type;
  
  assert( ftype != 0 );
  cdf = &ftype->caldefs;
  assert( cdf->icvt != 0 && cdf->cvt != 0 );
  tname = ftype->convert;
  if ( tname == 0 ) tname = ftype->decl->nameref;
  assert( tname->type == NMTYPE_TMTYPE );
  ttype = tname->u.tmtdecl;
  yscale = pow( 10.0, cdf->yscale );
  cdf->icvt->out_min = floor( cdf->cvt->out_min * yscale + .5 );
  cdf->icvt->out_max = floor( cdf->cvt->out_max * yscale + .5 );

  /* Print function entry code */
  ret_type = pick_a_type( ftype, ttype, cdf->icvt->out_min,
    cdf->icvt->out_max, NULL );
  gen_cvt_name( cdf->icvt, '(' );
  print_indent(NULL);
  fprintf( ofile, "/* doub icvt for %s -> %s */\n",
          ftype->decl->nameref->name, tname->name );
  fprintf( ofile, "%s %s %s x ) {\n", ret_type, cdf->icvt->fnpre, 
      ftype->decl->nameref->name );
  fprintf( ofile, "  return (%s) floor( %s x %s", ret_type,
          non_null( cdf->cvt->fnpre ), non_null( cdf->cvt->fnpost ));
  if ( cdf->yscale != 0 )
    fprintf( ofile, " * 1E%u", cdf->yscale );
  fprintf( ofile, " + .5 );\n}\n" );
}

/* gen_int_icvt() generates iconvert functions from calibration 
  specifications for integer types converting to fixed point 
  formats. The output type will be that of ftype->convert if that 
  is integral, otherwise an appropriate output type will be 
  selected based on the output range of the conversion. I no 
  longer make reference to the output format (except to determine 
  the scaling for fixed-point) in determine the range. That type 
  of checking belongs in the tcvt function.
*/
static void gen_int_icvt( struct tmtype *ftype ) {
  struct caldef *cdf;
  struct cvtfunc *cvs;
  struct nm *tname;
  struct tmtype *ttype;
  struct calibration *cal;
  double cvt_min, cvt_max;
  char *ret_type;
  struct intcnvl cl;
  
  assert( ftype != 0 );
  cdf = &ftype->caldefs;
  cvs = cdf->icvt; assert( cvs != 0 );

  /* Check the input range */
  type_range( ftype->decl->type, &cvt_min, &cvt_max );
  cvs->out_min = cvt_min;
  cvs->out_max = cvt_max;
  
  cal = cdf->cal;
  if ( cal == 0 ) return;
  tname = ftype->convert; assert( tname->type == NMTYPE_TMTYPE );
  ttype = tname->u.tmtdecl; assert(TYPE_NUMERIC(ttype->decl->type));
  
  /* generate chain of regions. Translates cvt_min/max from
     input_min/max to output_min/max
  */
  int_conv( cal, &cvt_min, &cvt_max, pow( 10.0, cdf->yscale ), &cl );
  cvs->out_min = cvt_min;
  cvs->out_max = cvt_max;

  /* Determine range of output */
  ret_type = pick_a_type( ftype, ttype, cvt_min, cvt_max, NULL );

  /* print data declarations */
  gen_cvt_name(cvs, '(');
  adjust_indent( -80 );
  print_indent(NULL);
  fprintf(ofile, "/* int icvt for %s -> %s */\n",
        ftype->decl->nameref->name, tname->name );
  fprintf(ofile, "static %s %s %s x) {", ret_type,
                cvs->fnpre, ftype->decl->nameref->name);
  fprintf(ofile, "\n  %s ov;\n", ret_type );
  print_indent(NULL);
  adjust_indent(0);

  /* generate code to convert to ttype */
  gen_itc_code( cl.n_regions, cl.first, "ov" );
  adjust_indent(2);
  print_indent( "\nreturn ov;" );
  adjust_indent(-2);
  print_indent( "\n}\n" );
}

/* gen_int_cvt() generates a floating point value using the fixed 
  point conversion designated by the icvt function.
*/
static void gen_int_cvt( struct tmtype *ftype ) {
  struct caldef *cdf;
  struct cvtfunc *cvs;

  assert( ftype != 0 );
  cdf = &ftype->caldefs;
  cvs = cdf->cvt;
  assert( cvs != 0 && cvs->next == 0 && cvs->fnpre == 0 );
  assert( cdf->icvt != 0 && cdf->icvt->next == 0 );
  if ( cdf->yscale == 0 ) {
    /* No scaling required */
    cvs->fnpre = cdf->icvt->fnpre;
    cvs->fnpost = cdf->icvt->fnpost;
    cvs->out_min = cdf->icvt->out_min;
    cvs->out_max = cdf->icvt->out_max;
  } else {
    char format[30];
    sprintf( format, "(%s", non_null( cdf->icvt->fnpre ) );
    cvs->fnpre = strdup( format );
    sprintf( format, "%s*1E-%u)",
              non_null( cdf->icvt->fnpost ), cdf->yscale );
    cvs->fnpost = strdup( format );
  }
}

/* gen_e_tcvt() generates text conversion for e-format
   output using the cvt function and dtoe.
*/
static void gen_e_tcvt( struct tmtype *ftype ) {
  char buf[40];
  struct pfmt pformat;
  struct caldef *cdf;

  assert( ftype != 0 );
  cdf = &ftype->caldefs;
  assert( cdf->cvt != 0 && cdf->tcvt != 0 );
  if ( ftype->txtfmt == 0 ) {
    compile_error( 2, "Type %s requires text format",
      ftype->decl->nameref->name );
    return;
  }
  parse_txtfmt( ftype->txtfmt, &pformat, NULL, NULL, NULL );
  if ( pformat.code != 'e' ) {
    compile_error( 2,
      "Conversions format %c for type %s not supported",
      pformat.code, ftype->decl->nameref->name );
    return;
  }
  if ( pformat.width == 0 ) {
    compile_error( 2,
      "Conversion format e for type %s requires explicit width",
      ftype->decl->nameref->name );
    return;
  }
  sprintf( buf, "dtoe(%s", non_null( cdf->cvt->fnpre ) );
  cdf->tcvt->fnpre = strdup( buf );
  sprintf( buf, "%s, %d, NULL )", non_null( cdf->cvt->fnpost ),
                  pformat.width );
  cdf->tcvt->fnpost = strdup( buf );
}

static struct {
  char *type;
  char *abbr;
} OT_abbr[] = {
  { "uint16_t", "US" },
  { "int16_t", "SS" },
  { "uint32_t", "UL" },
  { "int32_t", "SL" },
  { NULL, NULL }
};

typedef struct tcal_s {
  struct tcal_s *next;
  char *name;
} tcal;

static tcal *txtcals;

static char *generate_tfunc( char *in_type, unsigned int in_tcode,
                                        struct pfmt *pformat ) {
  char *tfname;

  { int i;
    char *abbr;
    char buf[80];
    tcal *tcals;
    
    for ( i = 0; OT_abbr[i].type != 0; i++ ) {
      if ( strcmp( OT_abbr[i].type, in_type ) == 0 ) break;
    }
    if ( OT_abbr[i].type == 0 )
      compile_error( 4, "pick_a_type return confusing" );
    abbr = OT_abbr[i].abbr;
    sprintf( buf, "%s_%u_%u_%c_%x", abbr,
      pformat->width, pformat->prec, pformat->code,
      pformat->flags & 0xF );

    /* Does the tfunc already exist? */
    for ( tcals = txtcals; tcals != 0; tcals = tcals->next ) {
      if ( strcmp( tcals->name, buf ) == 0 )
        return tcals->name;
    }
    tcals = new_memory( sizeof( tcal ) );
    tcals->next = txtcals;
    tcals->name = tfname = strdup( buf );
    txtcals = tcals;
  }

  { double fmt_min, fmt_max, input_min, input_max, yscale;
    char *ovtxt;
    int islong, i, p, radix, dot;
    static bool uc_digit_array_defined = false;
    static bool lc_digit_array_defined = false;
    static bool digit_offset_defined = false;

    /* generate the digit_array */
    if (!digit_offset_defined) {
      digit_offset_defined = true;
      fprintf(ofile, "%s\n", "#define DIGIT_OFFSET 15" );
    }
    if (isupper(pformat->code) && !uc_digit_array_defined) {
      uc_digit_array_defined = true;
      fprintf(ofile, "%s\n",
        "static const char uc_digit_array[] = "
        "\"FEDCBA9876543210123456789ABCDEF\";"
        );
    }
    if (!isupper(pformat->code) && !lc_digit_array_defined) {
      lc_digit_array_defined = true;
      fprintf(ofile, "%s\n",
        "static const char lc_digit_array[] = "
        "\"fedcba9876543210123456789abcdef\";"
        );
    }
    
    /* generate the text function */
    print_indent(NULL);
    fprintf(ofile, "const char *%s( %s x) {", tfname, in_type );
    adjust_indent( 2 );

    /* determine minimum and maximum output values based on format */
    format_range( pformat, &fmt_min, &fmt_max, &yscale);
  
    /* determine minimum and maximum input values based on type */
    type_range( in_tcode, &input_min, &input_max );

    if (input_min > fmt_min) fmt_min = input_min;
    if (input_max < fmt_max) fmt_max = input_max;

    /* print data declarations */
    fprintf(ofile, "\n  static char obuf[%d];", pformat->width+1);
    adjust_indent(0);
    if (fmt_min < 0) print_indent("int neg;");
    if (fmt_max > INT16_MAX || fmt_min < INT16_MIN) {
      print_indent("\nint iov;");
      islong = 1;
    } else islong = 0;
    ovtxt = "x";
    adjust_indent(-2); /* back to left margin */
    print_indent(NULL);
    adjust_indent(2);
  
    /* generate code to convert to text */
    if (input_min < fmt_min || input_max > fmt_max) {
      print_indent(NULL);
      fprintf(ofile, "if (");
      if (input_min < fmt_min) {
        fprintf(ofile, "%s < %d", ovtxt, (int32_t)floor(fmt_min+.5));
        if (input_max > fmt_max) fprintf(ofile, " || ");
      }
      if (input_max > fmt_max)
        fprintf(ofile, "%s > %d", ovtxt, (int32_t)floor(fmt_max+.5));
      fprintf(ofile, ") return(\"");
      for (i = pformat->width; i > 0; i--) fputc('*', ofile);
      fprintf(ofile, "\");");
      adjust_indent(0);
    }
    if (fmt_min < 0) {
      print_indent(NULL);
      fprintf(ofile, "neg = (%s < 0) ? 1 : 0;", ovtxt);
      adjust_indent(0);
    }
    dot = pformat->width; /* where to put a dot: default nowhere */
    i = pformat->width;
    p = pformat->width-2; /* where to start putting spaces if zero */
    if (pformat->flags & PF_ZERO) p = -1;
    print_indent(NULL);
    fprintf(ofile, "obuf[%d] = '\\0';", i--);
    switch (tolower(pformat->code)) {
      case 'f':
        radix = 10;
        dot -= pformat->prec+1;
        p = dot - 2;
        break;
      case 'u':
      case 'd': radix = 10; break;
      case 'b': radix = 2; break;
      case 'o': radix = 8; break;
      case 'x': radix = 16; break;
      case 'c':
        assert(i == 0);
        fprintf(ofile, "\n  obuf[0] = %s;", ovtxt);
        break;
      default: compile_error(4, "Unexpected code in gen_int_to_text");
    }
    for (; i >= 0; i--) {
      if (i == dot) fprintf(ofile, "\n  obuf[%d] = '.';", i);
      else {
        if (p < 0 && i == 0 && fmt_min < 0 ) {
          fprintf(ofile,
            "\n  if (neg) obuf[0] = '-'; else" );
        } else if (i <= p) {
          fprintf(ofile, "\n  if (%s == 0) ", ovtxt);
          if (fmt_min < 0) {
            fprintf(ofile, "{\n    if (neg) { obuf[%d] = '-'; goto ", i);
            if (i == 0) fprintf(ofile, "nospace");
            else fprintf(ofile, "space%d", i-1);
            fprintf(ofile, "; }\n    else ");
          }
          fprintf(ofile, "goto space%d;", i);
          if (fmt_min < 0) fprintf(ofile, "\n  }");
        }
        fprintf(ofile,
          "\n  obuf[%d] = %s_digit_array[(%s %% %d)+DIGIT_OFFSET];",
          i, (isupper(pformat->code) ? "uc" : "lc"), ovtxt, radix);
        fmt_max /= radix;
        if ( i > 0 ) {
          if (islong && fmt_max <= INT16_MAX) {
            fprintf(ofile, "\n  iov = %s/%d;", ovtxt, radix);
            ovtxt = "iov";
            islong = 0;
          } else fprintf(ofile, "\n  %s /= %d;", ovtxt, radix);
        }
      }
    }
    if (p >= 0) {
      fprintf(ofile, "\n  goto nospace;");
      for (i = p; i >= 0; i--)
        fprintf(ofile, "\n  space%d: obuf[%d] = ' ';", i, i);
      fprintf(ofile, "\n  nospace:");
    }
  
    /* close function */
    fprintf(ofile, "\n  return(obuf);");
    adjust_indent(-2);
    print_indent("}\n");
  }

  return tfname;
}

/* gen_int_tcvt() uses the icvt function and generates a 
   separate text conversion function.
*/
static void gen_int_tcvt( struct tmtype *ftype ) {
  struct caldef *cdf;
  struct nm *tname;
  struct tmtype *ttype;
  struct pfmt pformat;
  double fmt_min, fmt_max, yscale;
  char *in_type, *tfname;
  unsigned int in_tcode;
  
  assert( ftype != 0 );
  cdf = &ftype->caldefs;
  assert( cdf->icvt != 0 && cdf->tcvt != 0 );
  tname = ftype->convert;
  if ( tname == 0 ) tname = ftype->decl->nameref;
  assert( tname->type == NMTYPE_TMTYPE );
  ttype = tname->u.tmtdecl;
  assert(TYPE_NUMERIC(ttype->decl->type));
  if ( ftype->txtfmt == 0 ) {
    compile_error( 2, "Type %s requires text format",
              ftype->decl->nameref->name );
    return;
  }
  parse_txtfmt( ftype->txtfmt, &pformat, NULL, NULL, NULL );

  /* determine minimum and maximum output values based on format */
  format_range( &pformat, &fmt_min, &fmt_max, &yscale );
  
  /* determine the input type to the tfunc based on 
      fmt_min & fmt_max
  */
  if ( cdf->icvt->out_max > cdf->icvt->out_min ) {
    if (cdf->icvt->out_min > fmt_min) fmt_min = cdf->icvt->out_min;
    if (cdf->icvt->out_max < fmt_max) fmt_max = cdf->icvt->out_max;
  }
  in_type = pick_a_type( NULL, NULL, fmt_min, fmt_max, &in_tcode );
  if ( in_tcode == INTTYPE_DOUBLE ) {
    compile_error( 2,
      "Unable to select tfunc type for conversion of %s",
      ftype->decl->nameref->name );
    return;
  }

  /* Now use the type and format to generate the tfunc's name */
  tfname = generate_tfunc( in_type, in_tcode, &pformat );

  /* We've got the text function. Now we need to generate
     The glue between icvt and the text function
  */

  if ( cdf->icvt->out_min > cdf->icvt->out_max ||
        cdf->icvt->out_min < fmt_min || cdf->icvt->out_max > fmt_max ) {
    /*
      If we need to check the range, we need a temp variable using 
      the same type as returned by the icvt func. We can use 
      pick_a_type with the current values of cdf->icvt->out_min and 
      cdf->icvt->out_max, since that's how gen_int_icvt() does it.
    */
    char *var;

    gen_cvt_name(cdf->tcvt, '(');
    print_indent(NULL);
    fprintf( ofile,
            "/* %s) int tcvt %s -> %s */\n",
            cdf->tcvt->fnpre, ftype->decl->nameref->name,
            tname->name );
    var = "x";
    fprintf( ofile,
            "const char *%s%s %s) {\n", cdf->tcvt->fnpre,
            ftype->decl->nameref->name, var);
    compile_error(1,
      "Format \"%s\" may be narrow for input type %s",
      ftype->txtfmt, ftype->decl->nameref->name);
    if ( cdf->icvt->fnpre != 0 ) {
      char *tmp_type = pick_a_type( NULL, ttype, cdf->icvt->out_min, 
                                cdf->icvt->out_max, NULL );
      var = "iv";
      fprintf( ofile, "  %s %s;\n\n", tmp_type, var );
      fprintf( ofile, "  %s = %s x %s;\n", var,
              cdf->icvt->fnpre, cdf->icvt->fnpost );
    }
    fprintf( ofile, "  if ( " );
    if ( cdf->icvt->out_min > cdf->icvt->out_max ||
         cdf->icvt->out_min < fmt_min ) {
      fprintf( ofile, "%s < %.0lf ", var, fmt_min );
      if ( cdf->icvt->out_min > cdf->icvt->out_max ||
           cdf->icvt->out_max > fmt_max )
        fprintf( ofile, "|| " );
    }
    if ( cdf->icvt->out_min > cdf->icvt->out_max ||
         cdf->icvt->out_max > fmt_max )
      fprintf( ofile, "%s > %.0lf ", var, fmt_max );
    fprintf( ofile, ")\n\treturn \"" );
    { int i;
      for ( i = pformat.width; i > 0; i-- )
        putc( '*', ofile );
    }
    fprintf( ofile, "\";\n" );
    fprintf( ofile, "  return %s( %s );\n}\n", tfname, var );
  } else {
    /* No checking is required: we can simply generate
       tfunc(icvt(x))
    */
    int len;
    
    len = strlen( tfname ) + strlen( non_null(cdf->icvt->fnpre)) + 1;
    cdf->tcvt->fnpre = new_memory( len+1 );
    sprintf( cdf->tcvt->fnpre,
            "%s(%s", tfname, non_null( cdf->icvt->fnpre ) );
    if ( cdf->icvt->fnpost == 0 ) cdf->tcvt->fnpost = ")";
    else cdf->tcvt->fnpost = "))";
  }
}

static struct calibration *find_calibration( struct nm *fname,
                struct nm *tname ) {
  struct calibration *cal;

  assert(tname->type == NMTYPE_TMTYPE);
  for (cal = calibrations; cal != 0; cal = cal->next) {
    if (cal->type[0] == fname && cal->type[1] == tname)
      break;
  }
  return cal;
}

/* First pass identifies all the functions that are required.
   This function recurses.
*/
#define CV_UNUSED 0
#define CV_NEEDED 1
#define CV_EXPLICIT 2
#define CV_DEFINED 3
static void identify_calibrations( struct tmtype *ftype ) {
  int cvc, cvi, cvt, cal_needed = 0, cc;
  struct caldef *cdf;
  struct tmtype *ptype;

  assert( ftype != 0 );
  cdf = &ftype->caldefs;
  if ( ( cdf->convclass & CV_CLSFD ) == 0 )
    classify_conv( ftype );

  /* short-circuit if no conversions required */
  cvc = ( cdf->cvt != 0 ) ?
    ( ( cdf->cvt->fnpre == 0 && cdf->cvt->next == 0 ) ?
      CV_NEEDED : CV_EXPLICIT ) : CV_UNUSED;
  cvi = ( cdf->icvt != 0 ) ?
    ( ( cdf->icvt->fnpre == 0 && cdf->icvt->next == 0 ) ?
      CV_NEEDED : CV_EXPLICIT ) : CV_UNUSED;
  cvt = ( cdf->tcvt != 0 ) ?
    ( ( cdf->tcvt->fnpre == 0 && cdf->tcvt->next == 0  ) ?
      CV_NEEDED : CV_EXPLICIT ) : CV_UNUSED;

  if ( ( cdf->cvt != 0 && cdf->cvt->fnpre != 0 ) ||
       ( cdf->icvt != 0 && cdf->icvt->fnpre != 0 ) )
    cdf->convclass |= CV_HASCVT;
  if ( cdf->tcvt != 0 && cdf->tcvt->fnpre != 0 )
    cdf->convclass |= CV_HASTCVT;

  /* At this point we don't actually know whether we need a 
    calibration, but it is useful to find out what we know.
  */
  if ( ( cdf->convclass & CV_IDED ) == 0 ) {
    struct nm *fname, *tname;

    cdf->convclass |= CV_IDED;
    fname = ftype->decl->nameref;
    assert( fname->type == NMTYPE_TMTYPE );
    tname = ftype->convert;
    if ( ( cdf->convclass & CV_HASCVT ) == 0 ) {
      if ( tname != 0 )
        cdf->cal = find_calibration( fname, tname );
      if ( cdf->cal != 0 ) {
        cdf->convclass |= CV_HASCAL;
      } else {
        /* look to the parent type */
        ptype = ftype->decl->tm_type;
        if ( ptype != 0 && ptype->decl->size == ftype->decl->size 
              && ptype->convert == ftype->convert ) {
          /* We have a compatible parent type
             (we are not an aggregate based on the parent and we 
             convert to the same target type)
          */
          identify_calibrations( ptype );

          if ( ftype->txtfmt == 0 ||
                strcmp( ftype->txtfmt, ptype->txtfmt) == 0 ) {
            cdf->convclass |= CV_USEPRT;
            cdf->convclass = (cdf->convclass & ~CV_IX) | 
                        ( ptype->caldefs.convclass & CV_IX );
            /* can inherit all parent's functions. Must inherit
               parent's notion of whether it's cvt or icvt.
             */
          } else {
            int pcc;

            /* sort-of compatible (different format) */
            pcc = ptype->caldefs.convclass;
            if ( ( pcc & CV_IX) == 0 ) {
              if ( show(CONVERSIONS) )
                fprintf( vfile,
                  "Type %s can use cvt func of type %s\n",
                  fname->name, ptype->decl->nameref->name );
              cdf->convclass &= ~CV_IX;
              cdf->convclass |= CV_USEPCVT;
            } else if (pcc & CV_HASCAL) {
              /* compile_error() using inherited calibration? */
              if ( show(CONVERSIONS) )
                fprintf( vfile,
                  "Type %s can use calibration ( %s, %s )\n",
                  fname->name, ptype->decl->nameref->name,
                  tname->name );
              cdf->cal = ptype->caldefs.cal;
              cdf->convclass |= CV_HASCAL;
            }
          }
        }
      }
    }
  }

  if ( cvc != CV_NEEDED && cvi != CV_NEEDED && cvt != CV_NEEDED )
    return;

  cc = cdf->convclass;
  if ( cc & CV_USEPRT ) {
    if (show(CONVERSIONS))
      fprintf(vfile,
        "Type %s will use conversions of type %s\n",
        ftype->decl->nameref->name, ptype->decl->nameref->name );
    if ( cvi == CV_NEEDED ) {
      if ( ptype->caldefs.icvt == 0 )
        ptype->caldefs.icvt = mk_cvt_func( NULL, 0 );
      ftype->caldefs.icvt->next = ptype->caldefs.icvt;
      cvi = CV_DEFINED;
    }
    if ( cvc == CV_NEEDED ) {
      if ( ptype->caldefs.cvt == 0 )
        ptype->caldefs.cvt = mk_cvt_func( NULL, 0 );
      ftype->caldefs.cvt->next = ptype->caldefs.cvt;
      cvc = CV_DEFINED;
    }
    if ( cvt == CV_NEEDED ) {
      if ( ptype->caldefs.tcvt == 0 )
        ptype->caldefs.tcvt = mk_cvt_func( NULL, 0 );
      ftype->caldefs.tcvt->next = ptype->caldefs.tcvt;
      cvt = CV_DEFINED;
    }
    identify_calibrations( ptype );
  } else {
    if ( cc & CV_IX ) {
      if ( (cc & CV_HASCVT) == 0 &&
           (ftype->decl->type & INTTYPE_CHAR) )
        cc = (cdf->convclass |= CV_CX );
      if ( cvi == CV_UNUSED &&
            ( cvc == CV_NEEDED ||
              ( ( cc & CV_CX ) == 0 &&
                  cvt == CV_NEEDED ) ) )
        cvi = CV_NEEDED;
      if ( cvi == CV_NEEDED ) cal_needed = 1;
    } else if ( cc & CV_FIX ) {
      if ( cvi == CV_UNUSED && cvt == CV_NEEDED )
        cvi = CV_NEEDED;
      if ( cvc == CV_UNUSED && cvi == CV_NEEDED )
        cvc = CV_NEEDED;
      if ( cvc == CV_NEEDED ) cal_needed = 1;
    } else {
      if ( cvi != CV_UNUSED )
        compile_error( 2,
          "iconvert illegal with floating point type %s",
          ftype->decl->nameref->name );
      else if ( cvc == CV_UNUSED && cvt == CV_NEEDED )
        cvc = CV_NEEDED;
      if ( cvc == CV_NEEDED ) cal_needed = 1;
    }
  }

  if ( cvc == CV_NEEDED && cdf->cvt == 0 )
    cdf->cvt = mk_cvt_func( NULL, 0 );
  if ( cvi == CV_NEEDED && cdf->icvt == 0 )
    cdf->icvt = mk_cvt_func( NULL, 0 );
  if ( cvt == CV_NEEDED && cdf->tcvt == 0 )
    cdf->tcvt = mk_cvt_func( NULL, 0 );

  if ( cvc == CV_NEEDED && ( cc & CV_USEPCVT ) ) {
    if ( ptype->caldefs.cvt == 0 ) {
      ptype->caldefs.cvt = mk_cvt_func( NULL, 0 );
      identify_calibrations( ptype );
    }
    cdf->cvt->next = ptype->caldefs.cvt;
    cvc = CV_DEFINED;
    cc = ( cdf->convclass |= CV_HASCVT );
  }

  if ( cvc != CV_NEEDED && cvi != CV_NEEDED && cvt != CV_NEEDED )
    return;

  /* Now we know we need some non-trivial conversion */
  /* Determine yscale */
  cdf->yscale = 0;
  if ( ftype->txtfmt != 0 ) {
    struct pfmt pformat;
    parse_txtfmt( ftype->txtfmt, &pformat, NULL, NULL, NULL );
    if ( pformat.code == 'f' ) cdf->yscale = pformat.prec;
  }

  if ( cal_needed && ftype->convert != 0 &&
       ( cc & (CV_HASCAL | CV_HASCVT)) == 0 ) {
    compile_error( 2,
      "Calibration required between types %s and %s",
      ftype->decl->nameref->name, ftype->convert->name );
  }
}

/* TRUE if the cal is not yet generated (but needs to be) */
static int cal_ungend( struct cvtfunc *cvs ) {
  return cvs != 0 && cvs->fnpre == 0 && cvs->next == 0;
}

/* After all the required functions have been identified,
   generate_calibrations() sequences the actual function 
   generation.
*/
static void generate_calibrations( struct tmtype *ftype ) {
  struct caldef *cdf;
  int cc; /* convclass */

  assert( ftype != 0 );
  cdf = &ftype->caldefs;
  cc = cdf->convclass;
  if ( cc & CV_DEFD ) return;

  if ( ( cdf->cvt != 0 && cdf->cvt->next != 0 ) ||
       ( cdf->icvt != 0 && cdf->icvt->next != 0 ) ||
       ( cdf->tcvt != 0 && cdf->tcvt->next != 0 ) ) {
    /* generate the parent first, then we'll propogate */
    generate_calibrations( ftype->decl->tm_type );
    if ( cdf->cvt != 0 && cdf->cvt->next != 0 ) {
      cdf->cvt->fnpre = cdf->cvt->next->fnpre;
      cdf->cvt->fnpost = cdf->cvt->next->fnpost;
      cdf->cvt->out_min = cdf->cvt->next->out_min;
      cdf->cvt->out_max = cdf->cvt->next->out_max;
    }
    if ( cdf->icvt != 0 && cdf->icvt->next != 0 ) {
      cdf->icvt->fnpre = cdf->icvt->next->fnpre;
      cdf->icvt->fnpost = cdf->icvt->next->fnpost;
      cdf->icvt->out_min = cdf->icvt->next->out_min;
      cdf->icvt->out_max = cdf->icvt->next->out_max;
    }
    if ( cdf->tcvt != 0 && cdf->tcvt->next != 0 ) {
      cdf->tcvt->fnpre = cdf->tcvt->next->fnpre;
      cdf->tcvt->fnpost = cdf->tcvt->next->fnpost;
    }
  }

  if ( ( cc & CV_CX ) && cal_ungend( cdf->tcvt ) )
    generate_text_array( ftype );
  if ( cc & CV_IX ) {
    if ( cal_ungend( cdf->icvt ) ) gen_int_icvt( ftype );
    if ( cal_ungend( cdf->cvt  ) ) gen_int_cvt( ftype );
  } else {
    if ( cal_ungend( cdf->cvt  ) ) gen_doub_cvt( ftype );
    if ( cal_ungend( cdf->icvt ) ) gen_doub_icvt( ftype );
  }
  if ( cal_ungend( cdf->tcvt ) ) {
    if ( cc & CV_FIX ) gen_int_tcvt( ftype );
    else gen_e_tcvt( ftype );
  }
  cdf->convclass |= CV_DEFD;
}

void declare_convs(void) {
  struct nm *nr;

  for (nr = global_scope->names; nr != NULL; nr = nr->next) {
    if (nr->type == NMTYPE_TMTYPE)
      identify_calibrations( nr->u.tmtdecl );
  }
  for (nr = global_scope->names; nr != NULL; nr = nr->next) {
    if (nr->type == NMTYPE_TMTYPE)
      generate_calibrations( nr->u.tmtdecl );
  }
}

/* specify_conv() is responsible for placing the cvtfunc with the 
  source type definition.
*/
struct cvtfunc *specify_conv( struct tmtype *ftype,
        struct cvtfunc *cfn, int cfntype ) {
  struct cvtfunc **cfp;
  struct caldef *cdf;

  assert( cfn != NULL );
  cdf = &ftype->caldefs;
  
  switch ( cfntype ) {
    case CFLG_CVT: cfp = &cdf->cvt; break;
    case CFLG_ICVT: cfp = &cdf->icvt; break;
    case CFLG_TEXT: cfp = &cdf->tcvt; break;
    default: compile_error( 4, "Bad type in specify_conv" );
  }
  /* Check to see if this conversion is already specified */
  if ( *cfp == 0 ) {
    *cfp = cfn;
  } else {
    free_memory(cfn);
    cfn = *cfp;
  }
  return cfn;
}

/* Called from tmc.y. 
   s is a statement which should only contain a simple reference
   (which can only include pretext and a STATPC_REF).
   The STATPC_REF is replaced with a STATPC_CONVERT which points
   to the _REF.
   cflg flags whether or not this is a text conversion.
 */
void get_cfnc(struct sttmnt *s, int cflg) {
  struct statpc *spc;
  struct cvtfunc *cfn;
  struct nm *datum;
  struct tmtype *ftype;
  
  assert(s->last->type == STATPC_REF);
  if (!name_test(s->last->u.nameref, NMTEST_DATA)) {
    if ( cflg == CFLG_TEXT )
      compile_error(2, "Text conversion of non-TM datum %s",
                s->last->u.nameref->name);
    else
      compile_error(1, "Conversion of non-TM datum %s",
                s->last->u.nameref->name);
    return;
  }
  spc = newstpc(STATPC_CONVERT);
  spc->u.cvt.ref = s->last;
  spc->u.cvt.cfn = cfn = mk_cvt_func( NULL, 0 );
  assert(spc->u.cvt.ref->u.nameref != NULL);
  assert(spc->u.cvt.ref->next == NULL);

  /* Determine ftype */
  datum = spc->u.cvt.ref->u.nameref;
  ftype = nr_declarator(datum)->tm_type;
  if (ftype == NULL) {
    if ( cflg == CFLG_TEXT )
      compile_error(2, "Text conversion of datum %s: Not TM type",
                datum->name );
    else
      compile_error(1, "No conversion for %s: Not TM type", datum->name);
    return;
  }

  spc->u.cvt.cfn = specify_conv( ftype, cfn, cflg );

  if (s->first != s->last) {
    assert(s->first != NULL && s->first->next == s->last);
    s->first->next = NULL;
    s->last = s->first;
    catstatpc(s, spc);
  } else initstat(s, spc);
}

/* Now supporting null name */
struct cvtfunc *mk_cvt_func( char *name, char syntax ) {
  struct cvtfunc *cfn;
  int len;
  
  cfn = new_memory( sizeof(struct cvtfunc) );
  cfn->next = NULL;
  cfn->fnpre = NULL;
  cfn->fnpost = NULL;
  cfn->out_min = 0; cfn->out_max = -1;
  if ( name != 0 ) {
    assert( syntax == '(' || syntax == '[' );
    len = strlen( name );
    cfn->fnpre = new_memory( len + 2 );
    strcpy( cfn->fnpre, name );
    cfn->fnpre[len] = syntax;
    cfn->fnpre[len+1] = '\0';
    cfn->fnpost = ( syntax == '(' ) ? ")" : "]";
  }
  return cfn;
}

/* What is a useful classification?
   Need to know: is this an icvt type or a cvt type?
     icvt type if
       icvt is explicit (and hence ttype is integral) or
       ( cvt is *not* explicit and
         ( ttype is integral or fixed ) and ftype is integral and 
           (ftype not UL or ftype == ttype )
     cvt type otherwise
*/
void classify_conv( struct tmtype *ftype ) {
  struct nm *tname, *fname;
  struct tmtype *ttype;
  struct pfmt pformat;
  struct caldef *cdf;
  unsigned int ttypet, ftypet;
  
  assert( ftype != 0 && ftype->decl != 0 );
  cdf = &ftype->caldefs;
  
  fname = ftype->decl->nameref;
  assert( fname->type == NMTYPE_TMTYPE );
  assert( fname->u.tmtdecl == ftype );
  tname = ftype->convert;
  if ( tname == 0 ) tname = fname;
  assert( tname->type == NMTYPE_TMTYPE );
  ttype = tname->u.tmtdecl;
  cdf->convclass = CV_CLSFD;
  ftypet = ftype->decl->type;
  ttypet = ttype->decl->type;
  
  if ( TYPE_NUMERIC( ttypet ) ) {
    
    if ( ftype->txtfmt != 0 ) {
      parse_txtfmt( ftype->txtfmt, &pformat, NULL, NULL, NULL );
      switch ( pformat.code ) {
        case 'f': case 'u': case 'd': case 'b':
        case 'o': case 'x': case 'X': case 'c':
          cdf->convclass |= CV_FIX; break;
        case '\0': case 'e': case 'g': default:
          break;
      }
    }
    if ( ( cdf->icvt != 0 && cdf->icvt->fnpre != 0 ) ||
         ( ( cdf->cvt == 0 || cdf->cvt->fnpre == 0 ) &&
           TYPE_INTEGRAL( ftypet ) &&
           ( ( ftype == ttype ) || ( ! TYPE_ULONG( ftypet ) ) ) &&
              ( TYPE_INTEGRAL( ttypet ) ||
                  ( cdf->convclass & CV_FIX ) ) ) ) {
      cdf->convclass |= CV_IX;
    }
    if ( ( cdf->cvt == 0 || cdf->cvt->fnpre == 0 ) &&
         ( cdf->icvt == 0 || cdf->icvt->fnpre == 0 ) &&
         ( ftypet & INTTYPE_CHAR ) )
      cdf->convclass |= CV_CX;
  }
}
