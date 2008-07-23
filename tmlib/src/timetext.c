#include "tmctime.h"

char *timetext(long int t) {
  static char ts[10];
  int hh, h;
  
  if ( t < 0 ) {
	ts[0] = '-';
	t = -t;
  } else ts[0] = ' ';
  t = t % (24*3600L);
  ts[9] = '\0';
  hh = t % 60;
  h = hh % 10;
  ts[8] = h+'0';
  ts[7] = hh/10 + '0';
  ts[6] = ':';
  t /= 60;
  hh = t % 60;
  h = hh % 10;
  ts[5] = h+'0';
  ts[4] = hh/10 + '0';
  ts[3] = ':';
  t /= 60;
  hh = t % 100;
  h = hh % 10;
  ts[2] = h+'0';
  ts[1] = hh/10 + '0';
  return ts;
}
