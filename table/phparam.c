/* nparam.c Parametrized values for ntable (the text version ) */
#include "param.h"
#include "tablelib.h"

int ColSpace = 4;

dims_t CalcWordDims(char *text, int attr) {
  dims_t wdims;
  PhRect_t extent;
  char *font;

  switch (attr) {
    case 2: font = fieldfont; break;
	
  }
  tbl_ExtentText( font, text, &extent );
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
