/* nortlib.h include file for nortlib
 * $Log$
 * Revision 1.4  1993/02/15  17:58:49  nort
 * Having added a number of command interpreter functions for
 * client/server operation.
 *
 * Revision 1.3  1992/09/24  20:23:10  nort
 * With Command Queueing
 *
 * Revision 1.2  1992/09/09  18:45:23  nort
 * Latest version
 *
 * Revision 1.1  1992/08/25  15:31:42  nort
 * Initial revision
 *
 */
#ifndef _NORTLIB_H_INCLUDED
#define _NORTLIB_H_INCLUDED

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

pid_t nl_find_name(nid_t node, char *name); /* find_name.c */
pid_t nl_make_proxy(void *msg, int size); /* make_proxy.c */
pid_t find_DG(void); /* find_dg.c */
int send_DG(void *msg, int size); /* send_dg.c */
pid_t find_CC(int dg_ok); /* find_cc.c */
int send_CC(void *msg, int size, int dg_ok); /* send_cc.c */
int send_dascmd(int type, int value, int dg_ok); /* senddasc.c */
int reply_byte(pid_t sent_pid, unsigned char msg); /* repbyte.c */
int Soldrv_set_proxy(unsigned char selector, unsigned char ID, void *msg, int size); /* solprox.c */
int Soldrv_reset_proxy(unsigned char selector, unsigned char ID); /* solprox.c */

/* Command Interpreter Client (CIC) and Server (CIS) Utilities
   Message-level definition is in cmdalgo.h
 */
void cic_options(int argc, char **argv, char *def_prefix);
int cic_init(void);
int cic_query(char *version);
extern char ci_version[];
void cic_transmit(char *buf, int n_chars, int transmit);
int ci_sendcmd(const char *cmdtext, int test);
#define OPT_CIC_INIT "C:"
void ci_server(void);

/* tmcalgo (tma) support routines */
void tma_new_state(int partition, const char *name);
void tma_new_time(int partition, long int t1, const char *next_cmd);
int tma_time_check(int partition);
void tma_sendcmd(const char *cmd);
void tma_init_options(const char *hdr, int argc, char **argv);
#define OPT_TMA_INIT "r:pm"

#endif
