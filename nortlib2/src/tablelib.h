/* tablelib.h */
#ifndef TABLELIB_H
#define TABLELIB_H

#include <Pt.h>
#include <photon/Pf.h>

extern char *labelfont;
extern char *fieldfont;
void tbl_vert_sep( PtWidget_t *parent, int x, int y, int h );
void tbl_horiz_sep( PtWidget_t *parent, int x, int y, int w );
PtWidget_t *tbl_label_widget( PtWidget_t *window, char *text,
		int x, int y, char *font, PgColor_t color, int align );
PtWidget_t *tbl_label( PtWidget_t *window, char *text, int x, int y );
PtWidget_t *tbl_field( PtWidget_t *window, char *text, int x, int y );
PtWidget_t *tbl_window( char *title );

#endif

