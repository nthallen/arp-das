/* rational.h contains definitions required for rational numbers.
   Written 1-26-87
   Moved to ps February 25, 1988
   Stolen for data use May 1, 1991

   $Id$
   $Log$
   Revision 1.1  2008/07/03 15:31:02  ntallen
   Added rational.h

 * Revision 1.2  1994/11/22  14:55:40  nort
 * Changes for 32-bit
 *
 * Revision 1.1  1992/07/20  15:30:58  nort
 * Initial revision
 *
*/
#ifndef _RATIONAL_H
#define _RATIONAL_H

typedef struct {
  short int num;
  short int den;
} rational;

extern void rreduce(rational *);
extern void rplus(rational *, rational *, rational *);
extern void rminus(rational *, rational *, rational *);
extern void rtimes(rational *, rational *, rational *);
extern void rdivide(rational *, rational *, rational *);
extern void rtimesint(rational *a, short int b, rational *c);
extern void rdivideint(rational *a, short int b, rational *c);
int rcompare(rational *a, rational *b);
extern rational zero, one_half, one;
#endif
