/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name          stcpm - pattern match (unanchored)
*
* synopsis      length = stcpm(s,p,q);
*               int length;         length of match
*               char *s;            string being scanned
*               char *p;            pattern string
*               char **q;           points to matched string if successful
*
* description   This function scans the specified string to find the
*               first substring that matches the specified pattern.
*
* returns       length = 0 if no match
*                    = length of matching substring
*               *q = pointer to substring if successful
*
**/
/* $Revision$ $Date$ */

#include <string.h>
#include "port.h"

int stcpm(const char *str, char *pat,char **match) {
  int sx, ret;

  for (sx=0; ((ret=stcpma(&str[sx],pat)) == 0) && (str[sx]!='\0');sx++)
	  ;
  *match = (ret == 0)? NULL :(char *)&str[sx];
  return(ret);
}

