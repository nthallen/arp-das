/* nparam.c Parametrized values for ntable (the text version ) */
#include "param.h"
#include "tablelib.h"

#define MAX_FIELD_WIDTH 150

int ColSpace = 4;
int BaselineSkip = 15;

dims_t CalcWordDims(char *text, int attr) {
  dims_t wdims;
  PhRect_t extent;
  char *font;

  switch (attr) {
    case 2: font = tbl_fieldfont; break;
    default:
	case 4: font = tbl_labelfont; break;
  }
  tbl_ExtentText( font, text, &extent );
  wdims.Width.Space = extent.lr.x - extent.ul.x;
  wdims.Width.Glue = 0;
  wdims.Height.Space = extent.ul.y - extent.lr.y;
  wdims.Height.Glue = 0;
  if ( wdims.Height.Space < BaselineSkip )
    wdims.Height.Space = BaselineSkip;
  return wdims;
}

int DatumWidth(int ncols) {
  char buf[MAX_FIELD_WIDTH+1];
  int i;
  PhRect_t extent;

  if ( ncols > MAX_FIELD_WIDTH ) ncols = MAX_FIELD_WIDTH;
  for ( i = 0; i < ncols; i++ ) buf[i] = ' ';
  buf[ncols] = '\0';
  tbl_ExtentText( tbl_labelfont, buf, &extent );
  return extent.lr.x - extent.ul.x;
}

int DatumHeight(int nrows) {
  return nrows * BaselineSkip;
}

int RuleThickness = 5;
