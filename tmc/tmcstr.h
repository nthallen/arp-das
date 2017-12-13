/* tmcstr.h Defines structures used in parsing.
   $Log$
   Revision 1.8  1999/08/30 17:19:22  nort
   Changes for conversions and calibrations

 * Revision 1.7  1993/05/25  01:52:10  nort
 * Removed posttext from declrtor.
 *
 * Revision 1.6  1993/05/21  19:45:11  nort
 * Added State Variable Support
 *
 * Revision 1.5  1993/04/01  22:02:06  nort
 * Restructuring
 *
 * Revision 1.4  1993/03/29  04:21:28  nort
 * Added structure elements to support depend/else and
 * direct validations
 *
 * Revision 1.3  1992/08/07  13:37:00  nort
 * Alpha plus plus
 *
 * Revision 1.2  1992/07/21  20:35:42  nort
 * Alpha plus
 *
 * Revision 1.1  1992/07/20  15:30:58  nort
 * Initial revision
 *
 *******************************************************
 * Items notated with \*--*\ (mirrored) are marked for replacement by
 * general attributes or templates.
*/
#ifndef _TMCSTR_H
#define _TMCSTR_H

#ifndef _RATIONAL_H
  #error Must include rational.h before tmcstr.h
#endif

/* nm is the structure by which names are defined. The next field is
   used at definition to chain together the names in the current scope.
   If you need to create a list of names, you need to use a namelist.
 */
struct nm {
  struct nm *next;
  char *name;
  unsigned int type;
  union {
    struct dtm *ddecl; /* _DATUM */
    struct tmdtm *tmdecl; /* _TMDATUM */
    struct tmtype *tmtdecl; /* _TMTYPE */
    struct grpdef *grpd; /* _GROUP */
    struct statevar *stdecl; /* _STATE */
  } u;
};
#define NMTYPE_UNDEFINED 0
#define NMTYPE_TMTYPE 1
#define NMTYPE_DATUM 2
#define NMTYPE_TMDATUM 3
#define NMTYPE_GROUP 4
#define NMTYPE_DUMMY 5
#define NMTYPE_STATE 6

struct nmlst {
  struct nmlst *prev;
  struct nm *names;
};
struct deplst {
  struct deplst *next;
  struct nm *ref;
  unsigned int flag;
};
struct sltdep {
  struct sltdep *next;
  unsigned int per, row;
  unsigned int flag;
};
/* DEPL_ONCE applies to both deplst and sltdep flags
   DEPL_SPLIT applies to multi-slot data which is not "once" qualified
*/
#define DEPL_ONCE 1
#define DEPL_SPLIT 2
struct sttmnt {
  struct statpc *first;
  struct statpc *last;
};
struct statpc {
  struct statpc *next;
  unsigned int type;
  union {
    char *text; /* STATPC_TEXT */
    struct declrtor *decls; /* STATPC_DECLS */
    struct nm *nameref; /* STATPC_REF, _VALID, _INVALID */
    struct {
      struct deplst *vd;
      struct sltdep *sd;
      struct sttmnt stat;
      struct sttmnt else_stat;
      rational rate;
    } dep; /* _DEPEND */
    struct {
      struct nm *dummy; /* The dummy */
      struct nm *nameref; /* The dummy replacement */
      struct sttmnt stat; /* The statement */
    } defrule; /* STATPC_DEFRUL instance */
    struct sttmnt stat; /* _TLDECLS, _EXTRACT */
    struct valrec *valchk; /* _VALCHK */
    struct {
      struct cvtfunc *cfn;
      struct statpc *ref;
    } cvt; /* _CONVERT */
    struct statpc *cmn; /* _COMMON */
  } u;
};
#define STATPC_TEXT 0
#define STATPC_DECLS 1
#define STATPC_REF 2
#define STATPC_DEFRUL 3
#define STATPC_VALID 4
#define STATPC_INVALID 5
#define STATPC_DEPEND 6
#define STATPC_TLDECLS 7
#define STATPC_EXTRACT 8
#define STATPC_VALCHK 9
#define STATPC_ADDR 10
#define STATPC_CONVERT 11
#define STATPC_COMMON 12

/* tmalloc records all data pertaining to rate of TM data
   including location of the datum or group in the TM frame.
   Flags take TMDF_ values shown below. sltcw holds the actual
   chain of slot positions.
*/
struct tmalloc {
  struct tmalloc *next;
  unsigned int flags;
  struct nm *nameref;
  unsigned int size;
  rational rate;
  struct cw *sltcw;
};
#define TMDF_HOMEROW 1
#define TMDF_GRPMEM  2

/* valrec holds validation information. varname is a generated variable
   name. bitval is a mask which is applied to the specified variable
   on in/validation. funcname is currently unused, but was planned
   to provide optimization by grouping common validations into
   functions.
*/
struct valrec {
  struct valrec *next;
  char *varname;
  unsigned int bitval;
  char *funcname;
};

/* validator holds data on all items dependent on a particular datum.
   The chain of valrec's contain bit validation, and the valstat
   contains statements which don't have positional dependence but
   rather need to be placed at each validation. (These are candi-
   dates for functional optimization)
*/
struct validator {
  unsigned int flag;
  struct valrec *val;
  struct sttmnt valstat; /* direct validations */
};
/* DCLF_VALIDATE means this non-TM var requires validation.
   DCLF_VALID means the initial value is 'valid'
   DCLF_TLSET means initial value has been defined
*/
#define DCLF_VALIDATE 1
#define DCLF_VALID 2
#define DCLF_TLSET 4

/* declrtor is used for both type definitions and data definitions.
   If the type or datum is defined in terms of a tmtype, tm_type
   points to that definition.
*/
struct declrtor {
  struct declrtor *next;
  unsigned int type;
  unsigned int size;
  struct sttmnt typeparts;
  struct sttmnt decl; /* includes "pretext" after typeparts */
  struct nm *nameref; /* points back to the name this declares */
  struct tmtype *tm_type;
  unsigned short flag; /*--*/
  unsigned short address; /*--*/
};
/* DCLF_ADDRDEF means the address has been defined */
#define DCLF_ADDRDEF 1
#define DCLF_ARRAY   2

struct dtm {
  char *name;
  struct declrtor *decl;
  struct validator *vldtr;
};

struct tmdtm {
  char *name;
  struct declrtor *decl;
  struct tmalloc *tmdef;
  struct sttmnt collect;
};

struct grpdef {
  struct tmalloc *tmdef;
  struct nmlst *grpmems;
};

struct statevar {
  struct stdecl *next;
  unsigned short index;
  char *funcname;
  struct validator *vldtr;
};

struct stateset {
  struct stateset *next;
  char *funcname;
  unsigned short n_states;
  struct nmlst *states;
};

struct cvtfunc {
  struct cvtfunc *next; /* now points to parent's func */
  /* int flag; */
  char *fnpre;
  char *fnpost;
  double out_min, out_max;
};
#define CFLG_CVT 0
#define CFLG_TEXT 1
#define CFLG_ICVT 2

struct caldef {
  int convclass;
  struct calibration *cal;
  struct cvtfunc *cvt;
  struct cvtfunc *icvt;
  struct cvtfunc *tcvt;
  unsigned int yscale;
};
/* tmtype defines type declarations. This has been true only
   for TM type definitions, but I will remove the distinction
*/
struct tmtype {
  struct declrtor *decl;
  struct nm *dummy; /*--*/
  struct sttmnt collect; /*--*/
  struct nm *convert;
  struct caldef caldefs;
  char *txtfmt;
};
/* convclass conversion classification:
  CV_IX Integer Convert func is primary (else floating)
  CV_CX Character text function can be generated
  CV_FIX Output is fixed point
  CV_UK Unknown
*/
#define CV_IX 1
#define CV_CX 2
#define CV_FIX 4
#define CV_IDED 8
#define CV_UK 0x10
#define CV_CLSFD 0x20
#define CV_DEFD 0x80
#define CV_HASCAL 0x100
#define CV_HASCVT 0x200
#define CV_HASTCVT 0x400
#define CV_USEPRT 0x800
#define CV_USEPCVT 0x1000

/* Contains basic information about a type. If the type is a defined
   type, the tm_type points to the definition (to be implemented).
*/
struct typparts {
  unsigned int type;
  unsigned int size;
  struct sttmnt stat;
  struct tmtype *tm_type;
};
#define INTTYPE_CHAR 1
#define INTTYPE_INT 2
#define INTTYPE_LONG 4
#define INTTYPE_SHORT 8
#define INTTYPE_UNSIGNED 0x10
#define INTTYPE_FLOAT 0x20
#define INTTYPE_DOUBLE 0x40
#define INTTYPE_STRUCT 0x80
#define INTTYPE_UNION 0x100
#define INTTYPE_OTHER 0x200
#define TYPE_INTEGRAL(x) (!((x)&~0x1F))
#define TYPE_FLOATING(x) (!((x)&~0x60))
#define TYPE_NUMERIC(x)  (!((x)&~0x7F))
#define TYPE_ULONG(x) ((x)==0x14)

/* structs cw and slt define the "slots" of the TM frame into which
   datum fits. A slot defines the period and phase of a datum.
   A cw defines the column and width within a slot.
*/
struct cw {
  struct slt *slot; /* points back to parent */
  struct cw *next; /* i.e. next of this p&r */
  struct cw *dnext; /* next for this datum or group */
  struct nm *datum;
  unsigned int col;
  unsigned int width;
  char *home_row_text;
};

struct slt {
  struct slt *next;
  struct slt *calls;
  unsigned int per;
  unsigned int row;
  struct cw *cwl;
  struct sttmnt colacts;
  struct sttmnt extacts;
  struct valrec *val;
  char *cfunc; /* collection function name */
  char *efunc; /* extraction function name */
  char *bfunc; /* Combined function */
  char *ivfunc; /* invalidation function */
  unsigned int flag;
};
#define SLT_CCHKD 1
#define SLT_CNDD  2
#define SLT_ECHKD 4
#define SLT_ENDD  8
#define SLT_BCHKD 0x10
#define SLT_BNDD  0x20
#define SLT_PCHKD 0x40
#define SLT_NTHR  0x80
#define SLT_GEND  0x100
#endif
