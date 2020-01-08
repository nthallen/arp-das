/* tmc.h General include file for TM Compiler
*/
#include "compiler.h"
#define CO_COLLECT CO_CUSTOM
#define CO_NO_MAIN (CO_COLLECT<<1)

#define Collecting (compile_options&CO_COLLECT)
void declare_convs(void); /* calibr.c */
void new_scope(void); /* parsfunc.c */
void old_scope(void); /* parsfunc.c */
void generate_pcm(void); /* genpcm.c */
extern unsigned int BPMFmax, BPMFmin, Nrows, Ncols, Synchronous; /* genpcm.c */
extern unsigned long int SynchTolerance; /* genpcm.c */
extern unsigned int SynchPer; /* genpcm.c */
extern long int SynchDrift; /* genpcm.c */
extern unsigned int SynchValue, SynchInverted; /* genpcm.c */
extern unsigned int SecondsDrift; /* genpcm.c */
extern int TM_Data_Type; /* genpcm.c */
extern unsigned char md5_sig[16]; /* pdecls.c */
#ifdef _RATIONAL_H
  extern rational Rsynch; /* genpcm.c */
#endif
#define MFC_NAME "MFCtr"
#define SYNCH_NAME "Synch"
#define NULLFUNCNAME "nullfunc"
extern unsigned int show_tm; /* genpcm.c */
  #define show(x) (show_tm & SHOW_##x)
  #define setshow(x) show_tm |= SHOW_##x
  #define setnoshow(x) show_tm &= SHOW_##x
  #define SHOW_TM_PDEFS 1
  #define SHOW_TM_DEFS 2
  #define SHOW_TM_STEPS 4
  #define SHOW_TM_STATS 8
  #define SHOW_CONVERSIONS 0x10
void print_decls(void); /* pdecls.c */
void print_funcs(void); /* pfuncs.c */
void place_col(void); /* place.c */
void place_valid(void); /* place.c */
void place_ext(void); /* place.c */
void place_home(void); /* place.c */
void adjust_indent(int di); /* pdecls.c */
void print_indent(char *s); /* pdecls.c */
void print_pcm(void); /* pdecls.c */
void post_processing(void); /* postproc.c */
void add_ptr_proxy(char *type, char *name, int id); /* pointers.c */
void print_recv_objs(void); /* pointers.c */
void print_ptr_proxy(void); /* pointers.c */
void print_states(void); /* states.c */
struct nm;
void print_st_valid(struct nm *nr); /* states.c */
#if defined _STDIO_H_INCLUDED || ! defined __WATCOMC__
  extern FILE *ofile, *vfile, *dacfile, *addrfile;
  void ptr_hfile(char *filename, FILE *fp); /* pointers.c */
#endif
#ifdef _TMCSTR_H
  extern struct statpc *program; /* parsfunc.c */
  extern struct sttmnt initprog; /* parsfunc.c */
  extern struct sttmnt redrawprog; /* parsfunc.c */
  struct statpc *newstpc(unsigned int type); /* parsfunc.c */
  struct statpc *newstpctext(char *text); /* parsfunc.c */
  struct statpc *common_stat(struct statpc *sp); /* parsfunc.c */
  void catstatpc(struct sttmnt *s, struct statpc *spc); /* parsfunc.c */
  void catstattext(struct sttmnt *s, char *text); /* parsfunc.c */
  void initstat(struct sttmnt *s, struct statpc *spc); /* parsfunc.c */
  void catstat(struct sttmnt *s1, struct sttmnt *s2); /* parsfunc.c */
  struct deplst *newdeplst(char *name, int once); /* parsfunc.c */
  struct statpc *newdepend(void); /* parsfunc.c */
  extern struct nmlst *current_scope, *global_scope; /* parsfunc.c */
  struct nmlst *newnamelist(struct nmlst *prev, struct nm *names); /* parsfunc.c */
  struct nm *find_name(char *name, int declaring); /* parsfunc.c */
  struct nm *find_ref(char *name, int declaring); /* parsfunc.c */
  struct statpc *find_ref_spc(char *name, int declaring); /* parsfunc.c */
  void print_stat(struct statpc *spc); /* pdecls.c */
  struct validator *new_validator(void); /* parsfunc.c */
  void link_declarator(struct declrtor *decl, unsigned int type); /* parsfunc.c */
  struct declrtor *nr_declarator(struct nm *nameref); /* parsfunc.c */
  struct validator *nr_validator(struct nm *nameref); /* parsfunc.c */
  struct tmalloc *nr_tmalloc(struct nm * nameref); /* parsfunc.c */
  int name_test(struct nm *nameref, unsigned int testtype); /* parsfunc.c */
  #define NMTEST_DATA 0
  #define NMTEST_TYPE 1
  #define NMTEST_TMDATUM 2
  #define NMTEST_GROUP 3
  #define NMTEST_VALID 4
  #define NMTEST_DECLARATOR 5
  #define NMTEST_TMALLOC 6
  #define NMTEST_INVALID 7
  struct tmalloc *new_tmalloc(struct nm *nr); /* parsfunc.c */
  void clr_tmtype(struct tmtype *tmt);
  void print_decl(struct declrtor *decl);
  void print_valid(struct valrec *val, int valid); /* pfuncs.c */
  void print_vldtr(struct validator *vldtr, int valid); /* pfuncs.c */
  struct slt *get_slot(unsigned int p, unsigned int r); /* genpcm.c */
  struct slt *comp_slot(struct slt *slot, unsigned int p, unsigned int r); /* genpcm.c */
  void get_cfnc(struct sttmnt *s, int cflg); /* calibr.c */
  struct cvtfunc *mk_cvt_func( char *name, char syntax ); /* calibr.c */
  extern struct slt *sltlist;
  struct stateset *new_stateset(void); /* states.c */
  void add_state(struct stateset *set, char *name); /* states.c */

  /* yytype.h */
  struct st_un;

  /* decls.c */
  void int_type(struct typparts *tp, char *text, unsigned int type);
  unsigned int start_st_un(struct st_un *su, char *text,
        unsigned int type, unsigned int dclt);
  unsigned int end_st_un(struct typparts *tp, struct st_un *su,
        char *pre, struct sttmnt *decls, char *post);
  void set_typpts(struct typparts *tp, unsigned int type, unsigned int size,
        char *text, struct tmtype *tmt);
#endif
