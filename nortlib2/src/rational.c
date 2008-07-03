/* rational.c defines the various operations on rational numbers.
   Written February 25, 1988
   Modified November 20, 1988 not to pass structures.
*/
char rcsid_rational_c[] =
  "$Header$";

#include <limits.h>
#include "rational.h"
#include "nortlib.h"

rational zero = {0,1};
rational one_half = {1,2};
rational one = {1,1};

static void lreduce(long int num, long int den, rational *a) {
  int sign;
  long int r0, r1, r2;

  if (den < 0) { den = -den; num  = -num;}
  else if (den == 0) {
    a->num = 0;
    a->den = 1;
    return;
  }
  if (num < 0) { sign = 1; num = -num;}
  else if (num == 0) {
	a->num = 0;
    a->den = 1;
    return;
  } else sign = 0;
  if (num > den) { r0 = num; r1 = den;}
  else { r0 = den; r1 = num;}
  for (;;) {
    r2 = r0 % r1;
    if (r2 == 0) break;
    r0 = r1;
    r1 = r2;
  }
  num /= r1;
  den /= r1;
  if ( num < 0 || den < 0 )
	nl_error( 4, "Algorithmic error in rational lreduce" );
  if ( num > INT_MAX || den > INT_MAX ) {
	int resp;

	if ( nl_response > 2 ) resp = 4;
	else if ( nl_response > 0 ) resp = 1;
	else resp = 0;
	if ( resp )
	  nl_error( resp,
		"Cannot reduce %ld/%ld to short rational", num, den );
	num = 0;
	den = 1;
  }
  if (sign) num = -num;
  a->num = num;
  a->den = den;
  return;
}

void rreduce(rational *a) {
  lreduce((long) a->num, (long) a->den, a);
}

/* c may be one of a or b */
void rplus(rational *a, rational *b, rational *c) {
  lreduce(a->num*(long) b->den + a->den*(long) b->num,
		  a->den*(long) b->den, c);
}

void rminus(rational *a, rational *b, rational *c) {
  lreduce(a->num*(long) b->den - a->den*(long) b->num,
		  a->den*(long) b->den, c);
}

void rtimes(rational *a, rational *b, rational *c) {
  lreduce(a->num*(long) b->num, a->den*(long) b->den, c);
}

void rtimesint(rational *a, short int b, rational *c) {
  lreduce(a->num * (long) b, (long) a->den, c);
}

void rdivideint(rational *a, short int b, rational *c) {
  lreduce((long) a->num, a->den * (long) b, c);
}

void rdivide(rational *a, rational *b, rational *c) {
  lreduce(a->num * (long) b->den, b->num * (long) a->den, c);
}

/*+rational
<sort> rcompare

c = rcompare(a,b);
int c;
rational *a, *b;
rcompare returns -1 if a < b, 0 if a = b, and 1 if a > b.  It assumes both
a and b are in reduced form to the extent that the denominator is positive.
If this is not the case, an incorrect response is virtually assured.
-*/
int rcompare(rational *a, rational *b) {
  long int diff;

  diff = a->num * (long) b->den - a->den * (long) b->num;
  if (diff == 0) return(0);
  else if (diff < 0) return(-1);
  return(1);
}
