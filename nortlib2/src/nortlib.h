/* nortlib.h include file for nortlib
 *
 * More recent history can be found in revision control
 *
 * Revision 1.22  2012/02/27 01:03:50  ntallen
 * Added ascii_escape() function
 *
 * Revision 1.21  2009/03/02 17:11:30  ntallen
 * Added const to char* declarations as necessary.
 *
 * Revision 1.20  2008/08/16 14:44:26  ntallen
 * MSG codes from msg.h
 *
 * Revision 1.19  2007/05/09 17:14:47  ntallen
 * Delete functions from another library
 *
 * Revision 1.18  2001/12/04 15:06:05  nort
 * Debugging, etc.
 *
 * Revision 1.17  2001/10/11 16:34:41  nort
 * Added compiler.oui. Fixed compiler.h.
 *
 * Revision 1.16  2001/09/10 17:31:47  nort
 *
 * Patch to nl_error.c to match correct prototype
 * Patch to nortlib.h to exclude functions not ported QNX6
 * Add GNU support files
 *
 * Revision 1.15  2001/01/18 15:07:20  nort
 * Getcon functions, memo_shutdown() and ci_time_str()
 *
 * Revision 1.14  1999/06/18 18:04:05  nort
 * Added ci_settime (a long time ago?)
 * Added DigSelect() from dccc.c
 *
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

#include <stdio.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

int Skel_open(const char *name);
int Skel_copy(FILE *ofp, const char *label, int copyout);

extern int (*nl_error)(int level, const char *s, ...); /* nl_error.c */
int nl_err(int level, const char *s, ...); /* nl_error.c */
#ifdef va_start
  int nl_verror(FILE *ef, int level, const char *fmt, va_list args); /* nl_verr.c */
#endif

/* These codes are taken from the old msg.h */
#define MSG_DEBUG -2
#define MSG_EXIT -1
#define MSG_EXIT_NORM MSG_EXIT
#define MSG 0
#define MSG_PASS MSG
#define MSG_WARN 1
#define MSG_ERROR 2
#define MSG_FAIL MSG_ERROR
#define MSG_FATAL 3
#define MSG_EXIT_ABNORM 4
#define MSG_DBG(X) (MSG_DEBUG-(X))

extern int nl_debug_level; /* nldbg.c */
extern int nl_response; /* nlresp.c */
int set_response(int newval); /* nlresp.c */
#define NLRSP_DIE 3
#define NLRSP_WARN 1
#define NLRSP_QUIET 0

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

/* ascii_escape.c */
const char *ascii_escape(const char *ibuf);

#if defined(__QNXNTO__)
  #define OPTIND_RESET 1
#else
  #define OPTIND_RESET 0
#endif

#ifdef __cplusplus
};
#endif

#endif
