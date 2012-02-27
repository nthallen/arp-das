#include <stdio.h>
#include <stdlib.h>
#include "dtoe.h"

char rcsid_dtoe_c[] =
	"$Header$";
  
char *dtoe( double v, int width, char *obuf ) {
  static char sbuf[30];
  char *buf;
  int dec, sign, i, j;

  if ( obuf == 0 ) {
    obuf = sbuf;
    if ( width > 29 ) width = 29;
  }
  if ( width < 7 ) snprintf( obuf, width+1, "*******" );
  else {
    int nn;
    int prec = width-7;
    nn = snprintf( obuf, width+1, "% #*.*e", width, prec, v );
    if ( nn == width+1 ) {
      if ( prec > 0 ) {
        nn = snprintf( obuf, width+1, "% #*.*e", width-1, prec-1, v );
      } else {
        nn = snprintf( obuf, width+1, "% *.*e", width-1, prec, v );
      }
    }
    if ( nn != width ) obuf[0] = '!';
  }
  return obuf;
}
