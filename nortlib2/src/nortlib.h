/* nortlib.h include file for nortlib
 * $Log$
 * Revision 1.13  1996/04/02  19:06:44  nort
 * Put back some tma.h defs to support old version (temporarily)
 *
 * Revision 1.12  1996/04/02  18:35:56  nort
 * Pruned log, removed tmcalgo funcs to tma.h
 *
 * Revision 1.1  1992/08/25  15:31:42  nort
 * Initial revision
 *
 */
#ifndef _NORTLIB_H_INCLUDED
#define _NORTLIB_H_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <sys/types.h>

int Skel_open(char *name);
int Skel_copy(FILE *ofp, char *label, int copyout);

extern int (*nl_error)(int level, char *s, ...); /* nl_error.c */
int nl_err(int level, char *s, ...); /* nl_error.c */
#ifdef va_start
  int nl_verror(FILE *ef, int level, const char *fmt, va_list args); /* nl_verr.c */
#endif
extern int nl_debug_level; /* nldbg.c */
extern int nl_response; /* nlresp.c */
int set_response(int newval); /* nlresp.c */
#define NLRSP_DIE 3
#define NLRSP_WARN 1
#define NLRSP_QUIET 0

pid_t nl_find_name(nid_t node, const char *name); /* find_name.c */
char *nl_make_name(const char *base, int global); /* make_name.c */
pid_t nl_make_proxy(void *msg, int size); /* make_proxy.c */
pid_t find_DG(void); /* find_dg.c */
int send_DG(void *msg, int size); /* send_dg.c */
pid_t find_CC(int dg_ok); /* find_cc.c */
int send_CC(void *msg, int size, int dg_ok); /* send_cc.c */
int send_dascmd(int type, int value, int dg_ok); /* senddasc.c */
int reply_byte(pid_t sent_pid, unsigned char msg); /* repbyte.c */
int Soldrv_set_proxy(unsigned char selector, unsigned char ID, void *msg, int size); /* solprox.c */
int Soldrv_reset_proxy(unsigned char selector, unsigned char ID); /* solprox.c */
pid_t get_server_proxy(const char *name, int global, const char *cmd); /* cmdprox.c */

/* Command Interpreter Client (CIC) and Server (CIS) Utilities
   Message-level definition is in cmdalgo.h
 */
void cic_options(int argc, char **argv, const char *def_prefix);
int cic_init(void);
int cic_query(char *version);
extern char ci_version[];
void cic_transmit(char *buf, int n_chars, int transmit);
int ci_sendcmd(const char *cmdtext, int mode);
int ci_sendfcmd(int mode, char *fmt, ...);
void ci_settime( long int time );
#define OPT_CIC_INIT "C:p"
void ci_server(void); /* in nortlib/cis.c */
void cis_initialize(void); /* in cmdgen.skel or .cmd */
void cis_terminate(void);  /* in cmdgen.skel of .cmd */

/* tmcalgo (tma) support routines */
void tma_new_state(unsigned int partition, const char *name);
void tma_new_time(unsigned int partition, long int t1, const char *next_cmd);
void tma_hold(int hold);

/* R1 routines */
int tma_time_check(unsigned int partition);
void tma_sendcmd(const char *cmd);

/* guaranteed memory allocator, memlib.h subset.
 * Include memlib.h to obtain the full definition
 */
#ifndef MEMLIB_H_INCLUDED
  #define new_memory(x) nl_new_memory(x)
  #define free_memory(x) nl_free_memory(x)
  #define nl_strdup(x) nrtl_strdup(x)
#endif
void *nl_new_memory(size_t size);
void nl_free_memory(void *p);
char *nrtl_strdup(const char *s);

/* dccc.c */
unsigned short DigSelect( unsigned short cmd, unsigned short val );

#ifdef __cplusplus
};
#endif

#if defined __386__
#  pragma library (nortlib3r)
#elif defined __SMALL__
#  pragma library (nortlibs)
#elif defined __COMPACT__
#  pragma library (nortlibc)
#elif defined __MEDIUM__
#  pragma library (nortlibm)
#elif defined __LARGE__
#  pragma library (nortlibl)
# endif

#endif
