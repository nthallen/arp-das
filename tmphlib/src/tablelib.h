/* tablelib.h */
#ifndef TABLELIB_H
#define TABLELIB_H

#include <Pt.h>

#ifdef __cplusplus
extern "C" {
#endif

extern char *tbl_labelfont;
extern char *tbl_fieldfont;
void tbl_vert_sep( PtWidget_t *parent, int x, int y, int h, int dbl );
void tbl_horiz_sep( PtWidget_t *parent, int x, int y, int w, int dbl );
PtWidget_t *tbl_label_widget( PtWidget_t *window, const char *text,
		int x, int y, const char *font, PgColor_t color, int align );
PtWidget_t *tbl_label( PtWidget_t *window, const char *text, int x, int y );
PtWidget_t *tbl_field( PtWidget_t *window, const char *text, int x, int y, int w, int h );
PtWidget_t *tbl_window( const char *title, int w, int h );
void tbl_dispfield( PtWidget_t *field, const char *text );
int tbl_ExtentText( const char *font, const char *str, PhRect_t *extent );

#ifdef __cplusplus
};
#endif

#endif

