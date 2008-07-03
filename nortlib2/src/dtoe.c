#include <stdlib.h>
#include "dtoe.h"
char rcsid_dtoe_c[] =
	"$Header$";
  
char *dtoe( double v, int width, char *obuf ) {
  static char sbuf[30];
  char *buf;
  int dec, sign, i, j;

  if ( obuf == 0 ) obuf = sbuf;
  /* Subtract 6 from width to represent sign,decimal,E,sign,exponent
	 (assuming only two digits for the exponent */
  buf = ecvt( v, width-6, &dec, &sign );
  dec--;
  j = 0;
  obuf[j++] = sign ? '-' : ' ';
  obuf[j++] = buf[0];
  obuf[j++] = '.';
  for ( i = 1; buf[i] != 0; )
	obuf[j++] = buf[i++];
  obuf[j++] = 'E';
  if ( dec >= 0 ) obuf[j++] = '+';
  else {
	obuf[j++] = '-';
	dec = -dec;
  }
  obuf[j++] = dec / 10 + '0';
  obuf[j++] = dec % 10 + '0';
  obuf[j++] = '\0';
  return obuf;
}
