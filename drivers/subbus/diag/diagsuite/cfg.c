#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses/curses.h>
#include "cfg.h"
#ifdef __QNX__
  #if defined(__QNXNTO__)
    #include "port.h"
  #else
    #include "lat.h"
  #endif
#endif

int init_attrs(char *fname, int *attributes, int max) {
  FILE *fp;
  char buf[20];
  int i;

  for ( i = 0; i < max; i++ ) {
    attributes[i] = A_NORMAL | COLOR_PAIR(i+1);
    init_pair(i+1, COLOR_WHITE, COLOR_BLACK );
  }
  i = 0;
  fp = fopen(fname, "rb");
  if (fp != NULL) {
    for (; i < max; i++) {
      int attr, ifg, ibg;
      short fg, bg;
      if ( fgets(buf, 20, fp) == NULL ) break;
      if ( sscanf(buf, "%d%d%d", &attr, &ifg, &ibg) == 3 ) {
        attributes[i] = (attr & ~A_COLOR) | COLOR_PAIR(i+1);
        fg = ifg % COLORS;
        bg = ibg % COLORS;
        init_pair( i+1, fg, bg );
      }
    }
    fclose(fp); return(1);
  }
  return(0);
}


int save_attrs(char *name, int *attributes, int max) {
  FILE *fp;
  int i;
  fp = fopen(name, "wb");
  if (fp == NULL) return(0);
  for (i = 0; i < max; i++) {
    short foreground, background;
    pair_content(i+1, &foreground, &background);
    fprintf(fp, "%d %d %d\n", attributes[i] & ~A_COLOR, foreground, background);
  }
  fclose(fp);
  return(1);
}
