/*
 * File: hello.c
 */
#include "tablelib.h"

char *tbl_labelfont = "TitleFont10ba";
char *tbl_fieldfont = "FixedFont10";

void tbl_vert_sep( PtWidget_t *parent, int x, int y, int h, int dbl ) {
  PtArg_t args[5];
  PhPoint_t pos;
  PhDim_t dim;

  pos.x = x; pos.y = y;
  dim.w = 1; dim.h = h;
  PtSetArg( &args[0], Pt_ARG_POS, &pos, 0 );
  PtSetArg( &args[1], Pt_ARG_SEP_FLAGS, Pt_SEP_VERTICAL, Pt_SEP_ORIENTATION );
  PtSetArg( &args[2], Pt_ARG_DIM, &dim, 0 );
  PtSetArg( &args[3], Pt_ARG_SEP_TYPE,
            dbl ? Pt_DOUBLE_LINE : Pt_SINGLE_LINE, 0 );
  PtSetArg(&args[4], Pt_ARG_COLOR, Pg_CYAN, 0 );
  PtCreateWidget(PtSeparator, parent, 5, args );
}

void tbl_horiz_sep( PtWidget_t *parent, int x, int y, int w, int dbl ) {
  PtArg_t args[4];
  PhPoint_t pos;
  PhDim_t dim;

  pos.x = x; pos.y = y;
  dim.w = w; dim.h = 1;
  PtSetArg( &args[0], Pt_ARG_POS, &pos, 0 );
  PtSetArg( &args[1], Pt_ARG_DIM, &dim, 0 );
  PtSetArg( &args[2], Pt_ARG_SEP_TYPE,
            dbl ? Pt_DOUBLE_LINE : Pt_SINGLE_LINE, 0 );
  PtSetArg(&args[3], Pt_ARG_COLOR, Pg_CYAN, 0 );
  PtCreateWidget(PtSeparator, parent, 4, args );
}


PtWidget_t *tbl_label_widget( PtWidget_t *window, char *text,
		int x, int y, char *font, PgColor_t color, int align ) {
  PhPoint_t pos;
  PtArg_t args[8];

  pos.x = x; pos.y = y;
  PtSetArg(&args[0], Pt_ARG_TEXT_STRING, text, 0); 
  PtSetArg(&args[1], Pt_ARG_POS, &pos, 0 );
  PtSetArg(&args[2], Pt_ARG_TEXT_FONT, font, 0 );
  PtSetArg(&args[3], Pt_ARG_COLOR, color, 0 );
  PtSetArg(&args[4], Pt_ARG_HORIZONTAL_ALIGNMENT, align, 0 );
  PtSetArg(&args[5], Pt_ARG_MARGIN_WIDTH, 0, 0 );
  PtSetArg(&args[6], Pt_ARG_MARGIN_HEIGHT, 0, 0 );
  return PtCreateWidget(PtLabel, window, 7, args);
}

PtWidget_t *tbl_label( PtWidget_t *window, char *text, int x, int y ) {
  return tbl_label_widget( window, text, x, y, tbl_labelfont, Pg_GREEN, Pt_LEFT ); 
}

PtWidget_t *tbl_field( PtWidget_t *window, char *text, int x, int y, int w, int h ) {
  PtWidget_t *fld;
  PhDim_t dim;

  fld = tbl_label_widget( window, text, x, y, tbl_fieldfont, Pg_WHITE, Pt_RIGHT ); 
  dim.w = w; dim.h = h;
  PtSetResource( fld, Pt_ARG_DIM, &dim, 0 );
  return fld;
}

PtWidget_t *tbl_window( char *title, int width, int height ) {
  PtArg_t args[8];
  PtSetArg(&args[0], Pt_ARG_WINDOW_TITLE, title, 0); 
  PtSetArg(&args[1], Pt_ARG_WINDOW_MANAGED_FLAGS,
      Pt_FALSE, Ph_WM_MAX | Ph_WM_RESIZE );
  PtSetArg(&args[2], Pt_ARG_WINDOW_MANAGED_FLAGS,
      Pt_TRUE, Ph_WM_COLLAPSE );
  PtSetArg(&args[3], Pt_ARG_WIDTH, width, 0); 
  PtSetArg(&args[4], Pt_ARG_HEIGHT, height, 0); 
  PtSetArg(&args[5], Pt_ARG_WINDOW_RENDER_FLAGS,
      Pt_FALSE, Ph_WM_RENDER_MAX | Ph_WM_RENDER_RESIZE );
  PtSetArg(&args[6], Pt_ARG_WINDOW_RENDER_FLAGS,
      Pt_TRUE, Ph_WM_RENDER_COLLAPSE );
  PtSetArg(&args[7], Pt_ARG_FILL_COLOR, Pg_BLACK, 0); 
  return PtCreateWidget( PtWindow, Pt_NO_PARENT, 8, args );
}

void tbl_dispfield( PtWidget_t *field, const char *text ) {
  PtSetResource( field, Pt_ARG_TEXT_STRING, text, 0 );
}

int tbl_ExtentText( const char *font, const char *str, PhRect_t *extent ) {
  PhRect_t *extp = PfExtentText( extent, NULL, font, str, 0 );
  if ( extp == NULL ) return 0;
  else return 1;
}
