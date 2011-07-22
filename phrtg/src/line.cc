/** \file line.cc
 * Support routines for axes
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

plot_line::plot_line(plot_graph *parent_in, unsigned col, const char *name_in)
                : plot_obj(po_line, name_in ) {
  new_data = false;
  redraw_required = false;
  x_axis_trended = false;
  effective_visibility = true;
  parent = parent_in;
  parent_obj = parent;
  column = col;
  color = line_colors[col%N_COLORS];
}

plot_line *plot_line::new_line(plot_graph *parent, unsigned col) {
  char buf[80];
  snprintf(buf, 80, "%s[%u]", parent->name, col);
  return new plot_line(parent, col, buf);
}

plot_line::~plot_line() {
  if (destroying) return;
  destroying = true;

  nl_assert(parent->destroying); // Just my current assumption.

  // And therefore we don't have to mess with Current::Line
  // if (this == Current::Line)
  //   Current::Line = NULL;
  
  // We don't have to delete our widgets if the parent
  // widget is being deleted. The parent widget is the pane,
  // which corresponds to parent(data)->parent(axes)->parent(pane)
  clear_widgets();
  TreeItem->data = NULL;
  // The parent plot_graph object will be responsible
  // for deleting our tree elements and removing
  // us from its children
}

void plot_line::clear_widgets() {
  if (parent->parent->parent->destroying) {
    widgets.clear();
  } else {
    while (!widgets.empty()) {
      PtWidget_t *last = widgets.back();
      if (last != NULL) PtDestroyWidget(last);
      widgets.pop_back();
    }
  }
}

void plot_line::got_focus(focus_source whence) {
  if (this == Current::Line) return;
  plot_obj::got_focus(whence);
  Current::Line = this;
  Update_Line_Tab();
}

bool plot_line::check_limits( RTG_Range &Xr, RTG_Range &Yr ) {
  if ( ! visible ) return false;
  // plot_axes *ax = parent->parent;
  if (new_data) {
    new_data = false;
    redraw_required = true;
    Xrange.range_is_current = false;
    Yrange.range_is_current = false;
  }
  if ( Xr.range_required && ! Xrange.range_is_current )
    check_range = true;
  if ( ! Xr.range_required && Xrange.changed(Xr) )
    Yrange.range_is_current = false;
  if ( Yr.range_required && ! Yrange.range_is_current )
    check_range = true;
  if (check_range) {
    check_range = false;
    if ( Yr.range_required ) Yrange.range_required = true;
    if ( Xr.range_required ) Xrange.range_required = true;
    parent->variable->evaluate_range(column, Xrange, Yrange);
    return true;
  }
  if (Xr.range_required) Xr.update(Xrange);
  if (Yr.range_required) Yr.update(Yrange);
  return false;
}

void plot_line::set_line_color(PgColor_t rgb, Update_Source src) {
  if (color == rgb || redraw_required) return;
  color = rgb;
  for ( unsigned wn = 0; wn < widgets.size(); ++wn ) {
    PtSetResource(widgets[wn], Pt_ARG_COLOR, color, 0);
  }
  if (src != from_widget && this == Current::Line &&
      Current::Tab == Tab_Line) {
    PtSetResource(ABW_Line_Color, Pt_ARG_CS_COLOR, color, 0);
  }
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
    double X, Y;
    if (!var->get(i, column, X, Y))
      nl_error(4, "get failed for %s[%d]", name, i);
    idata[i].x = ax->X.evaluate(X);
    idata[i].y = ax->Y.evaluate(Y);
  }
  // PhDim_t minsize = { 0,0};
  PtArg_t args[2];
  PtSetArg( &args[1], Pt_ARG_COLOR, color, 0 );
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
  while (wn < widgets.size()) {
    PtWidget_t *last = widgets.back();
    if (last != NULL) PtDestroyWidget(last);
    widgets.pop_back();
  }
  return true;
}

/**
 * @return true if our visibility has changed.
 *
 * The graph will also check to see if we are now effectively
 * visible for deciding whether or not to check the variable
 * for updates. 
 * 
 *  Visibility Strategy for lines.
 * When a line is marked invisible, just move the widget off screen.
 * That way, if it is marked visible, it can be moved back onscreen
 * quickly. If new data arrives, there is no longer any value in
 * holding on to those widgets, so destroy them. The new_data flag
 * will remain set until the line is made visible, since all other
 * rendering routines will return quickly.
 * 
 * I realize that moving the widgets offscreen is overkill if the
 * pane or window is invisible, but it's quick and easier than
 * figuring out all the nuances.
 */
bool plot_line::check_for_updates( bool parent_visibility ) {
  std::vector<PtWidget_t*>::const_iterator pos;
  bool new_effective_visibility = new_visibility && parent_visibility;
  bool vis_change = false;
  if (effective_visibility) {
    if ( !new_effective_visibility ) {
      // hide the widgets
      PhPoint_t OffScreen = { -30000, 0 };
      for (pos = widgets.begin(); pos != widgets.end(); ++pos ) {
        PtSetResource(*pos, Pt_ARG_POS, &OffScreen, 0);
      }
      // parent->parent->X.data_range_updated = true;
      // parent->parent->Y.data_range_updated = true;
      vis_change = true;
    }
  } else if (new_effective_visibility) { // we have been invisible
    if (new_data || redraw_required) {
      // Data has changed, no need to hold on to these
      clear_widgets();
    } else {
      PhPoint_t OnScreen = { 0, 0 };
      for (pos = widgets.begin(); pos != widgets.end(); ++pos ) {
        PtSetResource(*pos, Pt_ARG_POS, &OnScreen, 0);
      }
    }
    // parent->parent->X.data_range_updated = true;
    // parent->parent->Y.data_range_updated = true;
    vis_change = true;
  }
  visible = new_visibility;
  effective_visibility = new_effective_visibility;
  return vis_change;
}

void plot_line::Update_Line_Tab() {
  if (Current::Tab != Tab_Line ) return;
  PtSetResource(ABW_Line_Name, Pt_ARG_TEXT_STRING, name, 0);
  PtSetResource(ABW_Line_Visible, Pt_ARG_FLAGS,
      visible ? Pt_TRUE : Pt_FALSE, Pt_SET);
  PtSetResource(ABW_Line_Color, Pt_ARG_CS_COLOR, color, 0);
  PtSetResource(ABW_Line_Variable_Name, Pt_ARG_TEXT_STRING,
      parent->variable->name, 0);
  PtSetResource(ABW_Line_Column, Pt_ARG_NUMERIC_VALUE, column, 0);
  PtSetResource(ABW_Line_Column, Pt_ARG_NUMERIC_MAX, parent->lines.size()-1, 0);
}
