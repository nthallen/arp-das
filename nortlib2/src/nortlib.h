/* nortlib.h include file for nortlib
 * $Log$
 */
#ifndef _NORTLIB_H_INCLUDED
#define _NORTLIB_H_INCLUDED
#ifndef FILENAME_MAX
  #error Must include stdio.h before nortlib.h
#endif

int Skel_open(char *name);
int Skel_copy(FILE *ofp, char *label, int copyout);

extern int (*nl_error)(unsigned int level, char *s, ...);
int nl_err(unsigned int level, char *s, ...);
#endif
