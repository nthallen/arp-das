/* ll_of_str.h Defines a linked list of string data type
 * $Log$
 * Revision 1.1  1994/11/22 14:56:13  nort
 * Initial revision
 *
 */
#ifndef _LLOFSTR_INCLUDED
#define _LLOFSTR_INCLUDED

struct llosleaf {
  struct llosleaf *next;
  char *text;
};

typedef struct {
  struct llosleaf *first;
  struct llosleaf *last;
} ll_of_str;

#ifdef __cplusplus
extern "C" {
#endif

void llos_enq(ll_of_str *ll, const char *str);
char *llos_deq(ll_of_str *ll);

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
