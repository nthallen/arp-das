/* pane.cc
  Support routines for the plot_pane module
  */
#include <ctype.h>
#include "ablibs.h"
#include "phrtg.h"
#include "abimport.h"
#include "nl_assert.h"

PtWidget_t *plot_pane::cache;

/*
 * This is the constructor for the plot_pane object. It can
 * be passed an existing PtPane widget (for the first pane
 * that exists when the window is created) or it can create
 * a new one.
 */
plot_pane::plot_pane( const char *name_in, plot_figure *figure)
        : plot_obj(po_pane, name_in) {
  PhDim_t min_dim = {20,20};
  
  parent = figure;
  parent_obj = figure;
  min_height = min_dim.h;
  synch_x = true;
  pane_color = Pg_BLACK;
  int n_panes = parent->panes.size();
  if ( n_panes == 0 ) ++n_panes;
  full_height = parent->dim.h/n_panes;

  PtArg_t args[2];
  PtSetArg( &args[0], Pt_ARG_ANCHOR_FLAGS, Pt_FALSE, Pt_TRUE);
  PtSetArg( &args[1], Pt_ARG_FLAGS, Pt_TRUE,
      Pt_HIGHLIGHTED|Pt_GETS_FOCUS);
  PtWidget_t *divider = ApGetWidgetPtr(parent->module, ABN_Figure_Div);
  widget = PtCreateWidget(PtPane, divider, 2, args );
  PtAddCallback(widget,Pt_CB_GOT_FOCUS,(PtCallbackF_t *)plot_obj::pt_got_focus,NULL);
  PtRealizeWidget(widget);

  PtSetResource(widget, Pt_ARG_FILL_COLOR, pane_color, 0 );
  PtSetResource(widget, Pt_ARG_POINTER, this, 0 );
  PtSetResource(widget, Pt_ARG_MINIMUM_DIM, &min_dim, 0 );

  parent->AddChild(this);
}

plot_pane::~plot_pane() {
  if ( destroying ) return;
  destroying = true;
  nl_assert(widget != NULL);
  while (!axes.empty()) delete axes.front();
  TreeItem->data = NULL;
  PtSetResource(widget, Pt_ARG_POINTER, NULL, 0 );
  parent->RemoveChild(this);
  widget = NULL;
}

/* plot_pane::AddChild(plot_axes *p);
 * Called from the child during construction
 * Responsible for adding the child to the end of
 * the list of children and adding it's TreeItem to 
 * the tree hierarchy
 */
void plot_pane::AddChild(plot_axes *ax) {
  if (axes.empty()) {
    PtTreeAddFirst(ABW_Graphs_Tab, ax->TreeItem, TreeItem);
  } else {
    PtTreeAddAfter(ABW_Graphs_Tab, ax->TreeItem, axes.back()->TreeItem);
  }
  axes.push_back(ax);
  if (current_child == NULL) current_child = ax;
}

/* void plot_pane::RemoveChild(plot_axes *ax);
 * Called from the child during its destruction.
 * Responsible for removing the child from the
 * list of children and possibly removing and
 * freeing it's TreeItem from the tree hierarchy.
 * This last step will be skipped if the figure
 * is in the process of destruction itself, since
 * the entire subtree will be removed and freed
 * when the figure is destroyed.
 */
void plot_pane::RemoveChild(plot_axes *ax) {
  nl_assert(ax != NULL);
  nl_assert(!axes.empty());
  if (current_child == ax) current_child = NULL;
  else { nl_assert(Current::Axes != ax); }
  axes.remove(ax);
  if (!destroying && current_child == NULL) {
    current_child = default_child();
    if (Current::Axes == ax) {
      if (current_child == NULL) Current::none(type);
      else current_child->got_focus(focus_from_parent);
    }
  }
}

plot_axes *plot_pane::CreateGraph(RTG_Variable_Data *var) {
  nl_assert(var != NULL);
  // Change this to use plot_axes() or plot_axes_diag()
  plot_axes *ax = new plot_axes(var->name, this);
  ax->CreateGraph(var);
  return ax;
}

void plot_pane::resized(PhDim_t *newdim ) {
  full_height = newdim->h;
  full_width = newdim->w;
  nl_assert( full_height >= min_height );
  nl_error(-2,"Pane %s resized to (%d,%d)", name, newdim->w, newdim->h );
  //### We'll need to do some math here to calculate
  //### the appropriate bounding boxes
  std::list<plot_axes*>::const_iterator ax;
  for ( ax = axes.begin(); ax != axes.end(); ++ax ) {
    (*ax)->resized(newdim);
  }
}

void plot_pane::got_focus(focus_source whence) {
  if (this == Current::Pane) return;
  plot_obj::got_focus(whence);
  Current::Pane = this;
  if (Current::Tab == Tab_X || Current::Tab == Tab_Y)
    Update_Axis_Pane();
}

/* This routine couples the specific widget (ABW_Pane_Name) with
 * The specific container it may fit within (Tab_X or Tab_Y)
 */
void plot_pane::rename(const char *text, Update_Source src) {
  plot_obj::rename(text, src);
  if (src == from_file && Current::Pane == this
      && (Current::Tab == Tab_X || Current::Tab == Tab_Y)) {
    PtSetResource(ABW_Pane_Name, Pt_ARG_TEXT_STRING,
        Current::Pane->name, 0);
  }
}
void plot_pane::set_bg_color(PgColor_t rgb, Update_Source src) {
  if (pane_color == rgb) return;
  pane_color = rgb;
  PtSetResource(widget, Pt_ARG_FILL_COLOR, pane_color, 0);
  if (src != from_widget && this == Current::Pane &&
      (Current::Tab == Tab_X || Current::Tab == Tab_Y)) {
    PtSetResource(ABW_Pane_Color, Pt_ARG_CS_COLOR, rgb, 0);
  }
}

bool plot_pane::render() {
  //###
  std::list<plot_axes*>::const_iterator pos;
  for (pos = axes.begin(); pos != axes.end(); pos++) {
    plot_axes *ax = *pos;
    if ( ax->render() ) return true;
  }
  return false;
}

plot_obj *plot_pane::default_child() {
  if (axes.empty()) return NULL;
  else return axes.front();
}

/* Visibility Strategy
 * When a pane is made invisible, we need to remove it from
 * the divider and stash it somewhere else. When it becomes
 * visible again, we need to reinsert it into the divider.
 */
bool plot_pane::check_for_updates(bool parent_visibility) {
  bool updates_required = false;
  if (visible && !new_visibility) {
    if ( cache == NULL ) {
      PtArg_t args[2];
      PhPoint_t cache_pos = { -30000, 0 };
      PtSetArg( &args[0], Pt_ARG_POS, &cache_pos, 0);
      PtSetArg( &args[1], Pt_ARG_FLAGS, Pt_TRUE,
          Pt_BLOCKED);
      cache = PtCreateWidget(PtWindow, Pt_NO_PARENT, 2, args );
    }
    PtReparentWidget( widget, cache);
    visible = new_visibility;
    parent->Adjust_Panes(-min_height);
    updates_required = true;
  } else if (!visible && new_visibility) {
    // Look through the parent's panes to figure out
    // where we fit.
    visible = true;
    std::list<plot_pane*>::const_iterator pos;
    plot_pane *prev_sib = NULL;
    bool saw_this = false;
    for ( pos = parent->panes.begin(); pos != parent->panes.end(); ++pos) {
      plot_pane *cur = *pos;
      PtWidget_t *divider = ApGetWidgetPtr(parent->module, ABN_Figure_Div);
      PtReparentWidget(widget, divider);
      if ( cur == this ) {
        if (prev_sib) {
          // we go in ahead prev_sib
          PtWidgetInsert( widget, prev_sib->widget, 0);
          break;
        } else saw_this = true;
      } else if (cur->visible) {
        if ( saw_this ) {
          // we go in behind cur
          PtWidgetInsert( widget, cur->widget, 1);
          break;
        } else prev_sib = cur;
      }
      PtRealizeWidget(widget);
    }
    parent->Adjust_Panes(min_height);
    updates_required = true;
  }
  std::list<plot_axes*>::const_iterator pos;
  for (pos = axes.begin(); pos != axes.end(); pos++) {
    plot_axes *ax = *pos;
    if ( ax->check_for_updates(visible && parent_visibility) )
      updates_required = true;
  }
  return updates_required;
}

void plot_pane::Update_Axis_Pane() {
  PtSetResource(ABW_Pane_Name, Pt_ARG_TEXT_STRING, name, 0);
  PtSetResource(ABW_Pane_Visible, Pt_ARG_FLAGS,
      visible ? Pt_TRUE : Pt_FALSE, Pt_SET);
  PtSetResource(ABW_Pane_Color, Pt_ARG_CS_COLOR, pane_color, 0);
}
