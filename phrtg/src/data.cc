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
  parent->AddChild(this);
  variable->AddGraph(this);
}

plot_data::~plot_data() {
  if (destroying) return;
  destroying = true;
  if (this == Current::Graph)
		Current::Graph = NULL;
  // while (first != NULL) delete first;
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

/* Like all render operations, this should accomplish a single
 * non-trivial task and return true, or determined that nothing
 * needs to be done and return false.
 */
bool plot_data::render() {
  if (new_data) {
    if (visible) {
      unsigned i = lines.size();
      for ( ; i < variable->ncols; ++i ) {
        lines.push_back(plot_line::new_line(this, i));
        if (i == 0) {
          PtTreeAddFirst(ABW_Graphs_Tab, lines[i]->TreeItem, TreeItem);
        } else {
          PtTreeAddAfter(ABW_Graphs_Tab, lines[i]->TreeItem, lines[i-1]->TreeItem);
        }
      }
      for ( i = 0; i < variable->ncols; ++i ) {
        lines[i]->new_data = true;
      }
      redraw_required = true;
    }
    new_data = false;
  }
  if ( redraw_required && visible ) {
    for ( unsigned i = 0; i < lines.size(); ++i ) {
      if (lines[i]->render()) return true;
    }
  }
  redraw_required = false;
  // nl_error(0, "Render graph %s", name);
  return false;
}

bool plot_data::check_for_updates() {
  return variable->check_for_updates();
}
