/* data.cc
  Support routines for axes
  */
#include <ctype.h>
#include "ablibs.h"
#include "phrtg.h"
#include "abimport.h"
#include "nl_assert.h"

plot_data::plot_data(RTG_Variable_Data *var, plot_axes *parent_in)
      : plot_obj(po_data, var->name) {
  variable = var;
  parent = parent_in;
  parent_obj = parent_in;
  new_data = true;
  axes_rescaled = false;
  parent->AddChild(this);
  variable->AddGraph(this);
  if (Current::Graph == NULL) got_focus(focus_from_parent);
}

plot_data::~plot_data() {
  if (destroying) return;
  destroying = true;
  while (!lines.empty()) {
    delete lines.back();
    lines.pop_back();
  }
  TreeItem->data = NULL;
  variable->RemoveGraph(this);
  variable = NULL;
  parent->RemoveChild(this);
}

void plot_data::got_focus(focus_source whence) {
  if (this == Current::Graph) return;
  plot_obj::got_focus(whence);
  Current::Graph = this;
  // Update any dialogs that require it
}

bool plot_data::check_limits( RTG_Variable_Range &Xr, RTG_Variable_Range &Yr ) {
  if (!visible) return false;
  if (new_data) {
    // Make sure we have line objects for each column
    unsigned i = lines.size();
    for ( ; i < variable->ncols; ++i ) {
      lines.push_back(plot_line::new_line(this, i));
      if (i == 0) {
        PtTreeAddFirst(ABW_Graphs_Tab, lines[i]->TreeItem, TreeItem);
      } else {
        PtTreeAddAfter(ABW_Graphs_Tab, lines[i]->TreeItem, lines[i-1]->TreeItem);
      }
      if (current_child == NULL) current_child = lines.back();
    }
    if (this == Current::Graph && Current::Line == NULL && current_child != NULL)
      current_child->got_focus(focus_from_parent);
    for ( unsigned i = 0; i < variable->ncols; ++i ) {
      lines[i]->new_data = true;
    }
    redraw_required = true;
    new_data = false;
  }
  if (this == Current::Graph && Current::Line == NULL && Current::Tab == Tab_Line)
    PtSetResource(ABW_ConsoleGroup, Pt_ARG_PG_CURRENT, "Graphs", 0);
  for ( unsigned i = 0; i < variable->ncols; ++i ) {
    if (lines[i]->check_limits(Xr, Yr)) return true;
  }
  return false;
}

/* Like all render operations, this should accomplish a single
 * non-trivial task and return true, or determined that nothing
 * needs to be done and return false.
 */
bool plot_data::render() {
  if (!visible) return false;
  if ( axes_rescaled ) {
    nl_assert(variable->ncols <= lines.size()); //should be set in check_limits()
    for ( unsigned i = 0; i < variable->ncols; ++i ) {
      lines[i]->redraw_required = true;
    }
    axes_rescaled = false;
    redraw_required = true;
  }
  if ( redraw_required ) {
    for ( unsigned i = 0; i < lines.size(); ++i ) {
      if (lines[i]->render()) return true;
    }
  }
  redraw_required = false;
  return false;
}

plot_obj *plot_data::default_child() {
  if (lines.empty()) return NULL;
  else return lines[0];
}

/* Visibility Strategy:
 * Basically leave it up to the lines.
 */
bool plot_data::check_for_updates(bool parent_visibility) {
  std::vector<plot_line*>::const_iterator pos;
  bool updates_required = false;
  visible = new_visibility;
  for (pos = lines.begin(); pos != lines.end(); ++pos) {
    if ((*pos)->check_for_updates(visible && parent_visibility))
      updates_required = true;
  }
  return variable->check_for_updates() || updates_required;
}