/* nparam.c Parametrized values for ntable (the text version ) */
#include <string.h>
#include "param.h"

int ColSpace = 1;

dims_t CalcWordDims(char *text, int attr) {
  dims_t wdims;

  attr = attr;
  wdims.Width.Space = strlen(text);
  wdims.Width.Glue = 0;
  wdims.Height.Space = 1;
  wdims.Height.Glue = 0;
  return wdims;
}

int DatumWidth(int ncols) {
  return ncols;
}

int DatumHeight(int nrows) {
  return nrows;
}

int RuleThickness = 1;
