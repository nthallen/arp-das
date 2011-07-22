/** \file axes.cc
 * Support routines for axes
 */
#include <ctype.h>
#include "ablibs.h"
#include "phrtg.h"
#include "abimport.h"
#include "nl_assert.h"

const int plot_axis::pane_overage = 0;

plot_axis::plot_axis() {
  XY = Axis_X;
  reverse_dim = false;
  reverse = false;
  // data_range_updated = false;
  // axis_range_updated = false;
  axis_limits_updated = false;
  axis_limits_trended = false;
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
  if ( range.range_updated ) {
    range.range_updated = false;
    if ( limits.limits_trend ) {
      if ( limits.changed(range) ) {
        axis_limits_trended = true;
      }
    } else if ( limits.limits_auto ) {
      if ( limits.changed(range) ) {
        set_scale();
      }
    }
  }
}

void plot_axis::set_scale() {
  if ( limits.max != limits.min ) {
    scalev = pixels/(limits.max-limits.min);
    clip_max = limits.min + 20000/scalev;
    clip_min = limits.min - 20000/scalev;
  } else {
    clip_max = limits.min + 1;
    clip_min = limits.min - 1;
    scalev = 0;
  }
  axis_limits_updated = true;
}

void plot_axis::set_scale(int pixel_span) {
  if (pixels != pixel_span-pane_overage) {
    pixels = pixel_span-pane_overage;
    set_scale();
  }
}

void plot_axis::set_scale(double min, double max) {
  limits.min = min;
  limits.max = max;
  set_scale();
}

short plot_axis::evaluate(scalar_t val) {
  if (val > clip_max) val = clip_max;
  else if (val < clip_min) val = clip_min;
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
    axis_limits_trended = false;
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
  long is_auto = limits.limits_auto ? Pt_TRUE : Pt_FALSE;
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
  detrended = false;
  inverted = false;
  psd_transformed = false;
  ph_transformed = false;
  parent->AddChild(this);
}

plot_axes::~plot_axes() {
  if (destroying) return;
    destroying = true;
  while (!graphs.empty()) delete graphs.front();
  TreeItem->data = NULL;
  parent->RemoveChild(this);
}

void plot_axes::AddChild(plot_graph *data) {
  if (graphs.empty()) {
    PtTreeAddFirst(ABW_Graphs_Tab, data->TreeItem, TreeItem);
  } else {
  	PtTreeAddAfter(ABW_Graphs_Tab, data->TreeItem, graphs.back()->TreeItem);
  }
  graphs.push_back(data);
  if (current_child == NULL) current_child = data;
}

void plot_axes::RemoveChild(plot_graph *data) {
  nl_assert(data != NULL);
  nl_assert(!graphs.empty());
  if (current_child == data) current_child = NULL;
  else { nl_assert(Current::Graph != data); }
  graphs.remove(data);
  if ( !destroying && current_child == NULL) {
    current_child = default_child();
    if (Current::Graph == data) {
      if (current_child == NULL) Current::none(type);
      else current_child->got_focus(focus_from_parent);
    }
  }
}

plot_graph *plot_axes::CreateGraph(RTG_Variable_Data *var) {
  nl_assert(var != NULL);
  plot_graph *graph = new plot_graph(var, this);
  graph->got_focus(focus_from_user);
  return graph;
}

void plot_axes::got_focus(focus_source whence) { // got_focus(gf_type whence)
  if (this == Current::Axes) return;
  plot_obj::got_focus(whence);
  Current::Axes = this;
  Update_Axis_Pane();
}

void plot_axes::Update_Axis_Pane() {
  std::list<plot_graph*>::const_iterator gr;

  if ( Current::Tab != Tab_X && Current::Tab != Tab_Y )
    return;
  PtSetResource(ABW_Axes_Name, Pt_ARG_TEXT_STRING, name, 0);
  PtSetResource(ABW_Axes_Visible, Pt_ARG_FLAGS,
      visible ? Pt_TRUE : Pt_FALSE, Pt_SET);

  /* Set detrend toggle only if all graphs are detrended */
  detrended = true;
  inverted = true;
  psd_transformed = true;
  ph_transformed = true;
  for ( gr = graphs.begin(); gr != graphs.end(); ++gr ) {
    if ((*gr)->variable->type != Var_Detrend )
      detrended = false;
    if ((*gr)->variable->type != Var_Invert )
      inverted = false;
    if ((*gr)->variable->type != Var_FFT_PSD )
      psd_transformed = false;
    if ((*gr)->variable->type != Var_FFT_Phase )
      ph_transformed = false;
  }
  PtSetResource(ABW_Detrend, Pt_ARG_FLAGS,
      detrended ? Pt_TRUE : Pt_FALSE, Pt_SET);
  PtSetResource(ABW_Invert, Pt_ARG_FLAGS,
      inverted ? Pt_TRUE : Pt_FALSE, Pt_SET);
  PtSetResource(ABW_PSD, Pt_ARG_FLAGS,
      psd_transformed ? Pt_TRUE : Pt_FALSE, Pt_SET);
  PtSetResource(ABW_Phase, Pt_ARG_FLAGS,
      ph_transformed ? Pt_TRUE : Pt_FALSE, Pt_SET);

  switch (Current::Tab) {
    case Tab_X:
      X.Update_Axis_Pane(this);
      break;
    case Tab_Y:
      Y.Update_Axis_Pane(this);
      break;
    default:
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
  std::list<plot_graph*>::const_iterator gr;
  RTG_Range Xr, Yr;
  bool lims_up;
  
  if (X.limits.limits_trend) {
    Yr.range_required = false;
    for ( gr = graphs.begin(); gr != graphs.end(); ++gr ) {
      plot_graph *grph = *gr;
      if (grph->check_limits(Xr, Yr)) return true;
    }
    if ( X.range.changed(Xr) )
      X.check_limits();
    if ( Y.limits.limits_auto ) {
      Yr.range_required = true;
      Xr.max = X.limits.max + X.limits.epoch;
      Xr.min = X.limits.min + X.limits.epoch;
      Xr.range_required = false;
      Xr.range_is_current = true;
      for ( gr = graphs.begin(); gr != graphs.end(); ++gr ) {
        plot_graph *grph = *gr;
        if (grph->check_limits(Xr, Yr)) return true;
      }
      if ( Y.range.changed(Yr) )
        Y.check_limits();
    }
  } else {
    if ( ! X.limits.limits_auto ) Xr.range_required = false;
    if ( ! Y.limits.limits_auto ) Yr.range_required = false;
    if ( Xr.range_required || Yr.range_required ) {
      for ( gr = graphs.begin(); gr != graphs.end(); ++gr ) {
        plot_graph *grph = *gr;
        if (grph->check_limits(Xr, Yr)) return true;
      }
      if ( X.range.changed(Xr) )
        X.check_limits();
      if ( Y.range.changed(Yr) )
        Y.check_limits();
    }
  }

  lims_up = X.axis_limits_updated || Y.axis_limits_updated;
  if (lims_up || trended) {
    for (gr = graphs.begin(); gr != graphs.end(); ++gr) {
      plot_graph *grph = *gr;
      if ( lims_up ) grph->axes_rescaled = true;
      if ( X.axis_limits_trended)
        grph->x_axis_trended = true;
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
  std::list<plot_graph*>::const_iterator gr;
  for (gr = graphs.begin(); gr != graphs.end(); ++gr) {
    plot_graph *grp = *gr;
    if ( grp->render() )
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
  std::list<plot_graph*>::const_iterator pos;
  visible = new_visibility;
  for (pos = graphs.begin(); pos != graphs.end(); pos++) {
    plot_graph *graph = *pos;
    if ( graph->check_for_updates(visible && parent_visibility) )
      updates_required = true;
  }
  return updates_required;
}

void plot_axes::schedule_range_check() {
  std::list<plot_graph*>::const_iterator pos;
  for (pos = graphs.begin(); pos != graphs.end(); pos++) {
    plot_graph *graph = *pos;
    graph->check_range = true;
  }
}

/**
 * ax->Detrend(value);
 * If value is non-zero, we walk through the graphs in these axes
 * and detrend each one using the current x-limits.
 * If value is zero, we undetrend.
 * @param value non-zero to assert detrend
 */
void plot_axes::Detrend(long value) {
  std::list<plot_graph*>::const_iterator pos;
  if (value) {
    detrended = true;
    if (X.limits.limits_empty) {
      nl_error(1,"Empty X-range: Skipping detrend");
    } else if (!X.limits.limits_current) {
      nl_error(1,"X-limits not current: Skipping detrend");
    } else {
      for (pos = graphs.begin(); pos != graphs.end(); pos++) {
        plot_graph *graph = *pos;
        RTG_Variable_Data *var = graph->variable;
        // I could prevent detrending a detrend here,
        // but there isn't much point. The UI makes
        // it difficult, so no need to belabor the point
        // here.
	RTG_Variable_Detrend *dt = RTG_Variable_Detrend::Create(
	    var, X.limits.min, X.limits.max);
	if ( dt != NULL ) {
	  graph->variable = dt;
	  dt->AddGraph(graph);
	  var->RemoveGraph(graph);
	  graph->rename(dt->name,from_widget);
	}
      }
    }
  } else {
    detrended = false;
    for (pos = graphs.begin(); pos != graphs.end(); pos++) {
      plot_graph *graph = *pos;
      RTG_Variable_Data *var = graph->variable;
      RTG_Variable_Data *src = var->Derived_From();
      if (src != NULL && var->type == Var_Detrend) {
        graph->variable = src;
        src->AddGraph(graph);
        var->RemoveGraph(graph);
	graph->rename(src->name,from_widget);
      }
      graph->new_data = true;
    }
  }
}

/**
 * ax->Invert(value);
 * If value is non-zero, we walk through the graphs in these axes
 * and invert each one. If value is zero, we undetrend.
 * @param value non-zero to assert detrend
 */
void plot_axes::Invert(long value) {
  std::list<plot_graph*>::const_iterator pos;
  if (value) {
    inverted = true;
    for (pos = graphs.begin(); pos != graphs.end(); pos++) {
      plot_graph *graph = *pos;
      RTG_Variable_Data *var = graph->variable;
      RTG_Variable_Invert *inv = RTG_Variable_Invert::Create(var);
      graph->variable = inv;
      inv->AddGraph(graph);
      var->RemoveGraph(graph);
      graph->rename(inv->name,from_widget);
    }
  } else {
    inverted = false;
    for (pos = graphs.begin(); pos != graphs.end(); pos++) {
      plot_graph *graph = *pos;
      RTG_Variable_Data *var = graph->variable;
      RTG_Variable_Data *src = var->Derived_From();
      if (src != NULL && var->type == Var_Invert) {
        graph->variable = src;
        src->AddGraph(graph);
        var->RemoveGraph(graph);
	graph->rename(src->name,from_widget);
      }
      graph->new_data = true;
    }
  }
}

/**
 * ax->PSD(value);
 * If value is non-zero, we walk through the graphs in these axes
 * and fft/psd each one. If value is zero, we revert to the source..
 * @param value non-zero to assert the forward transform
 */
void plot_axes::PSD(long value) {
  std::list<plot_graph*>::const_iterator pos;
  if (value) {
    psd_transformed = true;
    if (X.limits.limits_empty) {
      nl_error(1,"Empty X-range: Skipping psd");
    } else if (!X.limits.limits_current) {
      nl_error(1,"X-limits not current: Skipping psd");
    } else {
      for (pos = graphs.begin(); pos != graphs.end(); pos++) {
	plot_graph *graph = *pos;
	RTG_Variable_Data *var = graph->variable;
	RTG_Variable_PSD *psd =
	  RTG_Variable_PSD::Create(var, X.limits.min, X.limits.max);
	if ( psd != NULL ) {
	  graph->variable = psd;
	  psd->AddGraph(graph);
	  var->RemoveGraph(graph);
	  graph->rename(psd->name,from_widget);
	}
      }
      X.limits.limits_auto = true;
      Y.limits.limits_auto = true;
    }
  } else {
    psd_transformed = false;
    for (pos = graphs.begin(); pos != graphs.end(); pos++) {
      plot_graph *graph = *pos;
      RTG_Variable_Data *var = graph->variable;
      if ( var->type == Var_FFT_PSD ) {
	RTG_Variable_Data *src = var->Derived_From();
	if ( src != NULL && src->type == Var_FFT ) {
	  src = src->Derived_From();
	  if ( src != NULL ) {
	    graph->variable = src;
	    src->AddGraph(graph);
	    var->RemoveGraph(graph);
	    graph->rename(src->name,from_widget);
	  } else {
	    nl_error( 2, "Failed to locate PSD source" );
	  }
	} else {
	  nl_error( 2, "PSD source was not FFT" );
	}
      }
      graph->new_data = true;
      X.limits.limits_auto = true;
      Y.limits.limits_auto = true;
    }
  }
}

/**
 * ax->Phase(value);
 * If value is non-zero, we walk through the graphs in these axes
 * and fft/phase each one. If value is zero, we revert to the source..
 * @param value non-zero to assert the forward transform
 */
void plot_axes::Phase(long value) {
  std::list<plot_graph*>::const_iterator pos;
  if (value) {
    ph_transformed = true;
    if (X.limits.limits_empty) {
      nl_error(1,"Empty X-range: Skipping phase");
    } else if (!X.limits.limits_current) {
      nl_error(1,"X-limits not current: Skipping phase");
    } else {
      for (pos = graphs.begin(); pos != graphs.end(); pos++) {
	plot_graph *graph = *pos;
	RTG_Variable_Data *var = graph->variable;
	RTG_Variable_Phase *ph =
	  RTG_Variable_Phase::Create(var, X.limits.min, X.limits.max);
	if ( ph != NULL ) {
	  graph->variable = ph;
	  ph->AddGraph(graph);
	  var->RemoveGraph(graph);
	  graph->rename(ph->name,from_widget);
	}
      }
      X.limits.limits_auto = true;
      Y.limits.limits_auto = true;
    }
  } else {
    ph_transformed = false;
    for (pos = graphs.begin(); pos != graphs.end(); pos++) {
      plot_graph *graph = *pos;
      RTG_Variable_Data *var = graph->variable;
      if ( var->type == Var_FFT_Phase ) {
	RTG_Variable_Data *src = var->Derived_From();
	if ( src != NULL && src->type == Var_FFT ) {
	  src = src->Derived_From();
	  if ( src != NULL ) {
	    graph->variable = src;
	    src->AddGraph(graph);
	    var->RemoveGraph(graph);
	    graph->rename(src->name,from_widget);
	  } else {
	    nl_error( 2, "Failed to locate Phase source" );
	  }
	} else {
	  nl_error( 2, "Phase source was not FFT" );
	}
      }
      graph->new_data = true;
      X.limits.limits_auto = true;
      Y.limits.limits_auto = true;
    }
  }
}

plot_axes_diag::plot_axes_diag( const char *name_in, plot_pane *parent )
      : plot_axes(name_in, parent) {
  widget = NULL;
  Xpx = Ypx = 0;
}

plot_axes_diag::~plot_axes_diag() {
  if (widget) {
    PtDestroyWidget(widget);
    widget = NULL;
  }
  // Vector should be auto-deleted
}

const int plot_axes_diag::Nlvls = 7;

void plot_axes_diag::draw(int side, int x0, int y0, int dx, int dy) {
  int i = side * (Nlvls*2 + 1);
  int xp = x0;
  int yp = y0;
  for (int lvl = 0; lvl < Nlvls; ++lvl) {
    idata[i].x = xp;
    idata[i].y = yp;
    ++i;
    if (side & 1) yp += dy;
    else xp += dx;
    idata[i].x = xp;
    idata[i].y = yp;
    ++i;
    if (side & 1) xp += dx;
    else yp += dy;
  }
  if (side & 1) xp = x0;
  else yp = y0;
  idata[i].x = xp;
  idata[i].y = yp;
}

bool plot_axes_diag::render() {
  if (visible) {
    if (!widget || Xpx != X.pixels || Ypx != Y.pixels) {
      Xpx = X.pixels;
      Ypx = Y.pixels;
      int nptside = Nlvls*2+1;
      int npts = 4*nptside+1;
      int dx = Xpx/(Nlvls+1);
      int dy = Ypx/(Nlvls+1);
      idata.resize(npts);
      draw(0, 0, 0, dx, 1);
      draw(1,X.pixels-1,0,-1,dy);
      draw(2,X.pixels-1,Y.pixels-1,-dx,-1);
      draw(3,0,Y.pixels-1,1,-dy);
      idata[npts-1].x = 0;
      idata[npts-1].y = 0;
      PtArg_t args[2];
      PtSetArg( &args[1], Pt_ARG_COLOR, Pg_RED, 0 );
      PtSetArg( &args[0], Pt_ARG_POINTS, &idata[0], npts );
      if (widget) PtSetResources( widget, 2, args );
      else {
        widget = PtCreateWidget(PtPolygon, parent->widget, 2, args );
        PtRealizeWidget(widget);
      }
    }
  } else if (widget) {
    PtDestroyWidget(widget);
    widget = NULL;
  }
  return plot_axes::render();
}
