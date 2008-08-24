/**
*
*		Copyright 1988, 1989 by Lattice, Inc.
*
* name		stch_i - convert hexadecimal to integer
*
* synopsis	count = stch_i(p,r);
*		int count;	number of characters scanned
*		char *p;	input string
*		int *r;		output integer
*
* description	This function performs an anchored scan of the input
*		string to convert a hexadecimal value into an integer.
*		The scan terminates when a non-hex character is hit.
*		Valid hex characters are 0 to 9, A to F, and a to f.
*
* returns	count = 0 if input string does not begin with a hex value
*		      = number of characters scanned.
*
**/
/* $Revision$ $Date$ */

#include <ctype.h>
#include <string.h>
#include "port.h"

stch_i(p,r)
const char *p;
int *r;
{
int i,j,c;

i = j = 0;
for (c = *p; isxdigit(c); c = p[++j])
{
i <<= 4;
if(isdigit(c)) i |= c-'0';
if(isupper(c)) i |= c-'A'+10;
if(islower(c)) i |= c-'a'+10;
}
*r = i;
return(j);
}
