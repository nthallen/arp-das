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
  visible = true;
  new_data = false;
  axes_rescaled = false;
  parent->AddChild(this);
  variable->AddGraph(this);
}

plot_data::~plot_data() {
  if (destroying) return;
  destroying = true;
  if (this == Current::Graph)
		Current::Graph = NULL;
  // delete the lines
  while (!lines.empty()) {
    delete lines.back();
    lines.pop_back();
  }
  TreeItem->data = NULL;
  // PtSetResource(widget, Pt_ARG_POINTER, NULL, 0 );
  variable->RemoveGraph(this);
  variable = NULL;
  parent->RemoveChild(this);
  // widget = NULL;
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
    }
    for ( unsigned i = 0; i < variable->ncols; ++i ) {
      lines[i]->new_data = true;
    }
    redraw_required = true;
    new_data = false;
  }
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

bool plot_data::check_for_updates() {
  return variable->check_for_updates();
}
