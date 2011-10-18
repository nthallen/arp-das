/* nparam.c Parametrized values for ntable (the text version ) */
#include "err.h"
#include "param.h"
#include "tablelib.h"

#define MAX_FIELD_WIDTH 150

int ColSpace = 10;
int BaselineSkip = 15;

static int do_output;
static PtWidget_t *window;

void SetupPhoton( int preview ) {
  if (PtInit(NULL) == -1)
    message(DEADLY,"Unable to initialize Photon", 0, &curpos);
  do_output = preview;
}

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
  wdims.Width.Space = extent.lr.x - extent.ul.x + 1;
  wdims.Width.Glue = 0;
  wdims.Height.Space = extent.lr.y - extent.ul.y + 1;
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
  for ( i = 0; i < ncols; i++ ) buf[i] = '0';
  buf[ncols] = '\0';
  tbl_ExtentText( tbl_fieldfont, buf, &extent );
  return extent.lr.x - extent.ul.x;
}

int DatumHeight(int nrows) {
  return nrows * BaselineSkip;
}

int RuleThickness = 9;

void preview_window( PTG_OUTPUT_FILE f, const char *name, int w, int h ) {
  if (do_output) window = tbl_window( name, w, h );
}

void preview_label( PTG_OUTPUT_FILE f, const char *str, int r, int c ) {
  if (do_output) tbl_label( window, str, r, c );
}

void preview_field( PTG_OUTPUT_FILE f, int fldnum, int r, int c, int w, int h ) {
  if (do_output) {
    char buf[10];
    sprintf(buf, "%d", fldnum);
    tbl_field( window, buf, r, c, w, h );
  }
}

void preview_loop( PTG_OUTPUT_FILE f ) {
  if ( do_output) {
    PtRealizeWidget(window);
    PtMainLoop();
  }
}

void preview_vrule( PTG_OUTPUT_FILE f, int r, int c, int h, int dbl ) {
  if ( do_output ) {
    tbl_vert_sep( window, r, c, h, dbl );
  }
}

void preview_hrule( PTG_OUTPUT_FILE f, int r, int c, int w, int dbl ) {
  if ( do_output ) {
    tbl_horiz_sep( window, r, c, w, dbl );
  }
}

