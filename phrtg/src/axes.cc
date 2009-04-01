/* axes.cc
  Support routines for axes
  */
#include <ctype.h>
#include "ablibs.h"
#include "phrtg.h"
#include "abimport.h"
#include "nl_assert.h"

plot_axis::plot_axis() {
  XY = Axis_X;
  draw[0] = draw[1] = false;
  reserve_tick_space[0] = reserve_tick_space[1] = false;
  draw_ticks[0] = draw_ticks[1] = false;
  reserve_tick_label_space[0] = reserve_tick_label_space[1] = false;
  draw_tick_label[0] = draw_tick_label[1] = false;
  reserve_label_space[0] = reserve_label_space[1] = false;
  draw_label[0] = draw_label[1] = false;
  limit_auto = true;
  log_scale = false;
  reverse = false;
  min = max = 0.;
  pixels = 0;
  scalev = 0.;
  major_tick_len = 0; // positive outward, negative inward
  minor_tick_len = 0;
  label_height = 0; // same units has *_tick_len.
}

void plot_axis::set_scale() {
  if ( max != min ) {
	scalev = pixels/(max-min);
  } else scalev = 0;
}

void plot_axis::set_scale(int pixel_span) {
  pixels = pixel_span;
  set_scale();
}

void plot_axis::set_scale(float minv, float maxv) {
  min = minv;
  max = maxv;
  set_scale();
}

void plot_axis::set_scale(f_matrix *data) {
  float minv = 0, maxv = 0;
  int r, c, nr = data->nrows, nc = data->ncols;
  float *col;

  if ( nr > 0 && nc > 0 ) {
	minv = maxv = data->vdata[0];
	for ( c = 0; c < nc; c++ ) {
	  col = data->mdata[c];
	  for ( r = 0; r < nr; r++ ) {
	    if ( col[r] < minv ) minv = col[r];
	    else if ( col[r] > maxv ) maxv = col[r];
	  }
	}
  }
  set_scale( minv, maxv );
}

plot_axes::plot_axes( const char *name_in, plot_pane *pane ) : plot_obj(po_axes, name_in) {
  X.XY = Axis_X;
  Y.XY = Axis_Y;
  parent = pane;
  parent_obj = pane;
  visible = true;
  parent->AddChild(this);
}

plot_axes::~plot_axes() {
  if (destroying) return;
  destroying = true;
  if (this == Current::Axes)
	Current::Axes = NULL;
  while (!graphs.empty()) delete graphs.front();
  TreeItem->data = NULL;
  // PtSetResource(widget, Pt_ARG_POINTER, NULL, 0 );
  parent->RemoveChild(this);
  // widget = NULL;
}

void plot_axes::AddChild(plot_data *data) {
  if (graphs.empty()) {
    PtTreeAddFirst(ABW_Graphs_Tab, data->TreeItem, TreeItem);
  } else {
  	PtTreeAddAfter(ABW_Graphs_Tab, data->TreeItem, graphs.back()->TreeItem);
  }
  graphs.push_back(data);
}

void plot_axes::RemoveChild(plot_data *data) {
  nl_assert(data != NULL);
  nl_assert(!graphs.empty());
  if (current_child == data) current_child = NULL;
  graphs.remove(data);
}

void plot_axes::CreateGraph(RTG_Variable *var) {
  nl_assert(var != NULL);
  plot_data *graph = new plot_data(var, this);
  graph->got_focus(focus_from_user);
}

void plot_axes::got_focus(focus_source whence) { // got_focus(gf_type whence)
  if (this == Current::Axes) return;
  plot_obj::got_focus(whence);
  Current::Axes = this;
  // Update dialogs for axes
}
