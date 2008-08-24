#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#ifdef __QNX__
  #if defined(__QNXNTO__)
    #include "port.h"
  #else
    #include "lat.h"
  #endif
#endif

int init_attrs(char *fname, char *attributes, int max) {
  FILE *fp;
  char buf[20];
  int i;

  for ( i = 0; i < max; i++ )
    attributes[i] = A_NORMAL;
  i = 0;
  fp = fopene(fname, "rb",0);
  if (fp != NULL) {
    for (; i < max; i++) {
      if ( fgets(buf, 20, fp) == NULL ) break;
      attributes[i] = atoi(buf);
    }
    fclose(fp); return(1);
  }
  return(0);
}


int save_attrs(char *name, char *attributes, int max) {
  FILE *fp;
  int i;
  fp = fopen(name, "wb");
  if (fp == NULL) return(0);
  for (i = 0; i < max; i++) fprintf(fp, "%d\n", attributes[i]);
  fclose(fp);
  return(1);
}
