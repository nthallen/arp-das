/* current.cc
  Support routines for the Current object
  */
#include <ctype.h>
#include "ablibs.h"
#include "phrtg.h"
#include "abimport.h"
#include "nl_assert.h"

plot_figure *Current::Figure;
plot_pane *Current::Pane;
plot_axes *Current::Axes;
plot_data *Current::Graph;

void Current::none(plot_obj_type parent_type) {
  switch (parent_type) {
    case po_figure:
      nl_error(0, "No current pane");
      Current::Pane = NULL;
    case po_pane:
      nl_error(0, "No current axes");
      Current::Axes = NULL;
    case po_axes:
      nl_error(0, "No current graph");
      Current::Graph = NULL;
    case po_data:
      nl_error(0, "No current line");
      //Current::Line = NULL;
    default:
      break;
  }
}
