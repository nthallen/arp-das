/*
 * File: hello.c
 */
#include <Pt.h>
#include <photon/Pf.h>
#include "tablelib.h"

char *labelfont = "TitleFont10";
char *fieldfont = "FixedFont10";

void tbl_vert_sep( PtWidget_t *parent, int x, int y, int h ) {
  PtArg_t args[4];
  PhPoint_t pos = { x, y };
  PhDim_t dim = { 1, h };
  PtSetArg( &args[0], Pt_ARG_POS, &pos, 0 );
  PtSetArg( &args[1], Pt_ARG_SEP_FLAGS, Pt_SEP_VERTICAL, Pt_SEP_ORIENTATION );
  PtSetArg( &args[2], Pt_ARG_DIM, &dim, 0 );
  PtSetArg( &args[3], Pt_ARG_SEP_TYPE, Pt_SINGLE_LINE, 0 );
  PtCreateWidget(PtSeparator, parent, 4, args );
}

void tbl_horiz_sep( PtWidget_t *parent, int x, int y, int w ) {
  PtArg_t args[3];
  PhPoint_t pos = { x, y };
  PhDim_t dim = { w, 1 };
  PtSetArg( &args[0], Pt_ARG_POS, &pos, 0 );
  PtSetArg( &args[1], Pt_ARG_DIM, &dim, 0 );
  PtSetArg( &args[2], Pt_ARG_SEP_TYPE, Pt_SINGLE_LINE, 0 );
  PtCreateWidget(PtSeparator, parent, 3, args );
}


PtWidget_t *tbl_label_widget( PtWidget_t *window, char *text,
		int x, int y, char *font, PgColor_t color, int align ) {
  PhPoint_t pos;
  PtArg_t args[6];

  pos.x = x; pos.y = y;
  PtSetArg(&args[0], Pt_ARG_TEXT_STRING, text, 0); 
  PtSetArg(&args[1], Pt_ARG_POS, &pos, 0 );
  PtSetArg(&args[2], Pt_ARG_TEXT_FONT, font, 0 );
  PtSetArg(&args[3], Pt_ARG_FILL_COLOR, color, 0 );
  PtSetArg(&args[4], Pt_ARG_HORIZONTAL_ALIGNMENT, align, 0 );
  return PtCreateWidget(PtLabel, window, 5, args);
}

PtWidget_t *tbl_label( PtWidget_t *window, char *text, int x, int y ) {
  return tbl_label_widget( window, text, x, y, labelfont, Pg_WHITE, Pt_LEFT ); 
}

PtWidget_t *tbl_field( PtWidget_t *window, char *text, int x, int y ) {
  return tbl_label_widget( window, text, x, y, fieldfont, Pg_WHITE, Pt_RIGHT ); 
}

PtWidget_t *tbl_window( char *title ) {
  PtArg_t args[3];
  PtSetArg(&args[0], Pt_ARG_WINDOW_TITLE, title, 0); 
  PtSetArg(&args[1], Pt_ARG_WINDOW_MANAGED_FLAGS,
      Pt_FALSE, Ph_WM_MAX | Ph_WM_RESIZE );
  PtSetArg(&args[2], Pt_ARG_WINDOW_MANAGED_FLAGS,
      Pt_TRUE, Ph_WM_COLLAPSE );
  return PtCreateWidget( PtWindow, Pt_NO_PARENT, 3, args );
}

