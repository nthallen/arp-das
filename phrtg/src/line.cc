/* line.cc
  Support routines for axes
  */
#include <ctype.h>
#include "ablibs.h"
#include "phrtg.h"
#include "abimport.h"
#include "nl_assert.h"

static PgColor_t line_colors[] = {
  Pg_BLUE, Pg_GREEN, Pg_RED, Pg_YELLOW, Pg_MAGENTA,
  Pg_CYAN, Pg_DGREEN, Pg_DCYAN, Pg_DBLUE, Pg_BROWN,
  Pg_PURPLE, Pg_CELIDON
};
#define N_COLORS (sizeof(line_colors)/sizeof(PgColor_t))

const unsigned plot_line::pts_per_polygon = 500;

plot_line::plot_line(plot_data *parent_in, unsigned col, const char *name_in)
                : plot_obj(po_line, name_in ) {
  visible = true;
  new_data = false;
  redraw_required = false;
  parent = parent_in;
  parent_obj = parent;
  column = col;
  color = line_colors[col%N_COLORS];
}

plot_line *plot_line::new_line(plot_data *parent, unsigned col) {
  char buf[80];
  snprintf(buf, 80, "%s[%u]", parent->name, col);
  return new plot_line(parent, col, buf);
}

plot_line::~plot_line() {
  if (destroying) return;
  destroying = true;

  nl_assert(parent->destroying); // Just my current assumption.

  if (this == Current::Line)
    Current::Line = NULL;
  
  // We don't have to delete our widgets if the parent
  // widget is being deleted. The parent widget is the pane,
  // which corresponds to parent(data)->parent(axes)->parent(pane)
  if (parent->parent->parent->destroying) {
    widgets.clear();
  } else {
    while (!widgets.empty()) {
      PtWidget_t *last = widgets.back();
      PtDestroyWidget(last);
      widgets.pop_back();
    }
  }
  TreeItem->data = NULL;
  // The parent plot_data object will be responsible
  // for deleting our tree elements and removing
  // us from its children
}

void plot_line::got_focus(focus_source whence) {
  if (this == Current::Line) return;
  plot_obj::got_focus(whence);
  Current::Line = this;
  // Update any dialogs that require it
}

bool plot_line::check_limits( RTG_Variable_Range &Xr, RTG_Variable_Range &Yr ) {
  if ( ! visible ) return false;
  plot_axes *ax = parent->parent;
  if (new_data) {
    if (ax->X.limits.range_auto) {
      Xrange.range_required = true;
      Xrange.range_auto = true;
      Xrange.range_is_current = false;
      Xrange.range_is_empty = true;
    } else Xrange = ax->X.limits;
    if (ax->Y.limits.range_auto) {
      Yrange.range_required = true;
      Yrange.range_auto = true;
      Yrange.range_is_current = false;
      Yrange.range_is_empty = true;
    } else Yrange = ax->Y.limits;
    new_data = false;
    redraw_required = true;
    if (Xrange.range_required || Yrange.range_required) {
      parent->variable->evaluate_range(column, Xrange, Yrange);
      if (Xrange.range_auto)
        ax->X.data_range_updated = true;
      if (Yrange.range_auto)
        ax->Y.data_range_updated = true;
      return true;
    }
  }
  if (ax->X.limits.range_auto) Xr.update(Xrange);
  if (ax->Y.limits.range_auto) Yr.update(Yrange);
  return false;
}

bool plot_line::render() {
  if (!visible || !redraw_required) return false;
  // unrealize or delete all existing widgets
  // could simply replace their attributes, but
  // it's important not to change the PhPoint_t*
  // data out from under them, and a vector resize
  // could do that. Of course if we aren't resizing
  // that won't be a problem
  
  // for the moment, I will draw all the points all
  // the time, and let Photon clip. Hence the number
  // of points that need to be drawn is the number
  // of rows in the variable.

  plot_axes *ax = parent->parent;
  RTG_Variable_Data *var = parent->variable;
  redraw_required = false;
  unsigned npts = var->nrows;
  if (npts > idata.size()) {
    idata.resize(npts);
    nl_assert(idata.size() == npts);
  }
  // unsigned nw = (npts+pts_per_polygon-3)/(pts_per_polygon-1);
  for ( unsigned i = 0; i < npts; ++i ) {
    scalar_t X, Y;
    if (!var->get(i, column, X, Y))
      nl_error(4, "get failed for %s[%d]", name, i);
    idata[i].x = ax->X.evaluate(X);
    idata[i].y = ax->Y.evaluate(Y);
  }
  // PhDim_t minsize = { 0,0};
  PtArg_t args[2];
  //PtSetArg( &args[1], Pt_ARG_AREA, &ax->area, 0 );
  PtSetArg( &args[1], Pt_ARG_COLOR, color, 0 );
  //PtSetArg( &args[2], Pt_ARG_MINIMUM_DIM, &minsize, 0 );
  unsigned wn = 0;
  for ( unsigned i = 0; i < npts; i += pts_per_polygon - 1, ++wn ) {
    unsigned np = npts-i;
    if (np > pts_per_polygon) np = pts_per_polygon;
    PtSetArg( &args[0], Pt_ARG_POINTS, &idata[i], np );
    if (wn < widgets.size()) PtSetResources( widgets[wn], 2, args );
    else {
      widgets.push_back(PtCreateWidget(PtPolygon, ax->parent->widget, 2, args ));
      PtRealizeWidget(widgets.back());
    }
  }
  return true;
}
