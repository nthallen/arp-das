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
  reverse_dim = false;
  reverse = false;
  data_range_updated = false;
  axis_range_updated = false;
  axis_limits_updated = false;
  draw[0] = draw[1] = false;
  reserve_tick_space[0] = reserve_tick_space[1] = false;
  draw_ticks[0] = draw_ticks[1] = false;
  reserve_tick_label_space[0] = reserve_tick_label_space[1] = false;
  draw_tick_label[0] = draw_tick_label[1] = false;
  reserve_label_space[0] = reserve_label_space[1] = false;
  draw_label[0] = draw_label[1] = false;
  log_scale = false;
  pixels = 0;
  scalev = 0.;
  major_tick_len = 0; // positive outward, negative inward
  minor_tick_len = 0;
  label_height = 0; // same units has *_tick_len.
}

void plot_axis::check_limits() {
  if ( axis_range_updated ) {
    axis_range_updated = false;
    if ( limits.range_auto ) {
      if ( limits.changed(range) ) {
        set_scale();
      }
    }
  }
}

void plot_axis::set_scale() {
  if ( limits.max != limits.min ) {
    scalev = pixels/(limits.max-limits.min);
  } else scalev = 0;
  axis_limits_updated = true;
}

void plot_axis::set_scale(int pixel_span) {
  if (pixels != pixel_span) {
    pixels = pixel_span;
    set_scale();
  }
}

void plot_axis::set_scale(double min, double max) {
  limits.min = min;
  limits.max = max;
  set_scale();
}

short plot_axis::evaluate(scalar_t val) {
  int rv = int((val-limits.min)*scalev);
  if ( reverse xor reverse_dim )
    rv = pixels - rv;
  return rv;
}

bool plot_axis::render( plot_axes *axes ) {
  /* For the moment, we won't do anything here, so always return false.
   */
  if (axis_limits_updated) {
    // redraw the axes
    if (this == Current::Axis) {
      Tab_Name tab = (XY == Axis_X ? Tab_X : Tab_Y);
      if ( Current::Tab == tab ) {
        Update_Axis_Pane_Limits();
      }
    }
    axis_limits_updated = false;
    // return true;
  }
  return false;
}

plot_obj *plot_axes::default_child() {
  if (graphs.empty()) return NULL;
  else return graphs.front();
}

void plot_axis::Update_Axis_Pane(plot_axes *parent) {
  Current::Axis = this;
  long is_auto = limits.range_auto ? Pt_TRUE : Pt_FALSE;
  PtSetResource(ABW_Auto_Scale, Pt_ARG_FLAGS, is_auto, Pt_SET);
  PtSetResource(ABW_Limit_Min, Pt_ARG_FLAGS, is_auto,
      Pt_GHOST|Pt_BLOCKED );
  PtSetResource(ABW_Limit_Max, Pt_ARG_FLAGS, is_auto,
      Pt_GHOST|Pt_BLOCKED );
  PtSetResource(ABW_Apply_Limits, Pt_ARG_FLAGS, is_auto,
      Pt_GHOST|Pt_BLOCKED );
  Update_Axis_Pane_Limits();
}

void plot_axis::Update_Axis_Pane_Limits() {
  double val;
  val = limits.min;
  PtSetResource(ABW_Limit_Min, Pt_ARG_NUMERIC_VALUE, &val, 0);
  val = limits.max;
  PtSetResource(ABW_Limit_Max, Pt_ARG_NUMERIC_VALUE, &val, 0);
}

void plot_axis::Clear_Axis_Pane() {
  PtSetResource(ABW_Axes_Name, Pt_ARG_TEXT_STRING, "", 0);
  PtSetResource(ABW_Axes_Visible, Pt_ARG_FLAGS, Pt_FALSE, Pt_SET);
  PtSetResource(ABW_Auto_Scale, Pt_ARG_FLAGS, Pt_FALSE, Pt_SET);
  Current::Axis = NULL;
}

plot_axes::plot_axes( const char *name_in, plot_pane *pane ) : plot_obj(po_axes, name_in) {
  Y.XY = Axis_Y;
  Y.reverse_dim = true;
  X.set_scale(pane->full_width);
  Y.set_scale(pane->full_height);
  parent = pane;
  parent_obj = pane;
  parent->AddChild(this);
  if (Current::Axes == NULL) got_focus(focus_from_parent);
}

plot_axes::~plot_axes() {
  if (destroying) return;
    destroying = true;
  if (this == Current::Axes)
    Current::Axes = NULL;
  while (!graphs.empty()) delete graphs.front();
  TreeItem->data = NULL;
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
  if (current_child == NULL) current_child = data;
}

void plot_axes::RemoveChild(plot_data *data) {
  nl_assert(data != NULL);
  nl_assert(!graphs.empty());
  if (current_child == data) current_child = NULL;
  graphs.remove(data);
}

void plot_axes::CreateGraph(RTG_Variable_Data *var) {
  nl_assert(var != NULL);
  plot_data *graph = new plot_data(var, this);
  graph->got_focus(focus_from_user);
}

void plot_axes::got_focus(focus_source whence) { // got_focus(gf_type whence)
  if (this == Current::Axes) return;
  plot_obj::got_focus(whence);
  Current::Axes = this;
  if (Current::Tab == Tab_X)
    Update_Axis_Pane(Axis_X);
  else if (Current::Tab == Tab_Y)
    Update_Axis_Pane(Axis_Y);
}

void plot_axes::Update_Axis_Pane(Axis_XY ax) {
  PtSetResource(ABW_Axes_Name, Pt_ARG_TEXT_STRING, name, 0);
  PtSetResource(ABW_Axes_Visible, Pt_ARG_FLAGS,
      visible ? Pt_TRUE : Pt_FALSE, Pt_SET);
  switch (ax) {
    case Axis_X:
      X.Update_Axis_Pane(this);
      break;
    case Axis_Y:
      Y.Update_Axis_Pane(this);
      break;
  }
}

void plot_axes::resized(PhDim_t *newdim) {
  //### This will get more complicated when we have
  //### to account for axes and labels and stuff
  X.set_scale(newdim->w);
  Y.set_scale(newdim->h);
}

bool plot_axes::check_limits() {
  std::list<plot_data*>::const_iterator gr;
  RTG_Variable_Range Xr, Yr;
  for ( gr = graphs.begin(); gr != graphs.end(); ++gr ) {
    if ((*gr)->check_limits(Xr, Yr)) return true;
  }

  if (X.limits.range_auto && X.data_range_updated) {
    X.data_range_updated = false;
    if (X.range.changed(Xr))
      X.axis_range_updated = true;
  }
  if (Y.limits.range_auto && Y.data_range_updated) {
    Y.data_range_updated = false;
    if (Y.range.changed(Yr))
      Y.axis_range_updated = true;
  }

  /* Now check to see if the limits need to be updated */
  X.check_limits();
  Y.check_limits();
  if (X.axis_limits_updated || Y.axis_limits_updated) {
    for (gr = graphs.begin(); gr != graphs.end(); ++gr) {
      (*gr)->axes_rescaled = true;
    }
  }
  return false;
}

/* This routine couples the specific widget (ABW_Axes_Name) with
 * The specific container it may fit within (Tab_X or Tab_Y)
 */
void plot_axes::rename(const char *text, Update_Source src) {
  plot_obj::rename(text, src);
  if (src == from_file && Current::Axes == this
      && (Current::Tab == Tab_X || Current::Tab == Tab_Y)) {
    PtSetResource(ABW_Axes_Name, Pt_ARG_TEXT_STRING,
        Current::Axes->name, 0);
  }
}

bool plot_axes::render() {
  /* First update the axes data range if any of the
   * graphs have updated their range
   */
  if (! visible ) return false;

  if ( check_limits() ) return true;
  if ( X.render(this) ) return true;
  if ( Y.render(this) ) return true;
  std::list<plot_data*>::const_iterator gr;
  for (gr = graphs.begin(); gr != graphs.end(); ++gr) {
    if ( (*gr)->render() )
      return true;
  }
  return false;
}

/* Visibility Strategy
 * Use same strategy as lines for hiding axes, i.e.
 * keep track of effective visibility
 * on switch to invisibility, move widgets off screen
 * on axes redraw while invisible.
 * For the moment, this means do nothing.
 */
bool plot_axes::check_for_updates(bool parent_visibility) {
  bool updates_required = false;
  std::list<plot_data*>::const_iterator pos;
  visible = new_visibility;
  for (pos = graphs.begin(); pos != graphs.end(); pos++) {
    plot_data *graph = *pos;
    if ( graph->check_for_updates(visible && parent_visibility) )
      updates_required = true;
  }
  return updates_required;
}
