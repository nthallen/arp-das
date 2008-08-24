/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		stcpma - pattern match (anchored)
*
* synopsis	length = stcpma(s,p);
*		int length;	length of match
*		char *s;	string being scanned
*		char *p;	pattern string
*
* description	This function scans the specified string to determine
*		if it begins with a substring that matches the specified
*		pattern.
*
* returns	length = 0 if no match
*		       = length of matching substring
*
**/
/* $Revision$ $Date$ */

#include <string.h>
#include "port.h"

int stcpma(const char *s, const char *p) {
  int sx,savesx,px,pc,m,n,i;

  for(sx = px = 0; (m = pc = p[px]) != '\0'; px++, sx++)
        {
        n = p[px+1];
        if(pc == '?') m = 0;
        if((pc == '\\') && (n != '\0'))
                {
                m = p[++px];
                n = p[px+1];
                 }
        if((pc == '+') || (pc == '*'))
                {
                n = pc;
                m = 0;
                }
        else if((n == '+') || (n == '*')) ++px;
        if(n == '+')
                {
                if(s[sx] == '\0') return(0);
                if(m == 0) sx++;
                else if(m != s[sx++]) return(0);
                }
        if((n == '+') || (n == '*'))
                {
                savesx = sx;
                if(m == 0) while(s[sx] != '\0') sx++;
                else while(s[sx] == m) sx++;
                if(p[px+1] == '\0') return(sx);
                for(;sx >= savesx;sx--) {
		  i = stcpma( &s[sx], &p[px+1] );
		  if (i) return(sx+i);
                }
                return(0);
                }
        if((m != 0)&&(m != s[sx])) return(0);
        if((m == 0)&&(s[sx] == '\0'))
                {
                while(p[px] == '?') px++;
                if(p[px] == '\0') return(sx);
                return(0);
                }
        }
  return(sx);
}

