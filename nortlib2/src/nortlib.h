/* nortlib.h include file for nortlib
 * $Log$
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

extern int (*nl_error)(unsigned int level, char *s, ...);
int nl_err(unsigned int level, char *s, ...);
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
extern char ci_version[];
void cic_transmit(char *buf, int n_chars, int transmit);
int ci_sendcmd(char *cmdtext, int test);
#define OPT_CIC_INIT "C:"
void ci_server(void);

#endif
