/* data.cc
  Support routines for axes
  */
#include <ctype.h>
#include "ablibs.h"
#include "phrtg.h"
#include "abimport.h"
#include "nl_assert.h"

plot_data::plot_data(RTG_Variable *var, plot_axes *parent_in)
	: plot_obj(po_data, var->name) {
  variable = var;
  parent = parent_in;
  parent_obj = parent_in;
  first = last = NULL;
  visible = true;
  parent->AddChild(this);
  variable->AddGraph(this);
}

plot_data::~plot_data() {
  if (destroying) return;
  destroying = true;
  if (this == Current::Graph)
		Current::Graph = NULL;
  while (first != NULL) delete first;
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
