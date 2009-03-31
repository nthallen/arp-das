/* data.cc
  Support routines for axes
  */
#include <ctype.h>
#include "ablibs.h"
#include "phrtg.h"
#include "abimport.h"
#include "nl_assert.h"

plot_data *Current::Graph;

plot_data::plot_data(RTG_Variable *var, plot_axes *parent_in)
	: plot_obj(po_data, var->name) {
  variable = var;
  parent = parent_in;
  visible = true;
}

plot_data::~plot_data() {
	
}

void plot_data::got_focus() {
  if (this == Current::Graph) return;
  // plot_obj::got_focus(whence);
  Current::Pane = parent->parent;
  Current::Figure = parent->parent->parent;
  nl_error(0, "Graph Got Focus: %s", name);
  nl_assert(TreeItem != NULL);
  if ( !(TreeItem->gen.list.flags&Pt_LIST_ITEM_SELECTED)) {
	PtTreeSelect(ABW_Graphs_Tab, TreeItem);
  }
  // all of the above handled by plot_obj::got_focus(whence);
  Current::Graph = this;
  Current::Axes = parent;
}
