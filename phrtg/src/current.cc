/** \file current.cc
 * Support routines for the Current object
 */
#include <ctype.h>
#include "ablibs.h"
#include "phrtg.h"
#include "abimport.h"
#include "nl_assert.h"

plot_figure *Current::Figure;
plot_pane *Current::Pane;
plot_axes *Current::Axes;
plot_axis *Current::Axis;
plot_graph *Current::Graph;
plot_line *Current::Line;
RTG_Variable_Data *Current::Variable;
plot_obj *Current::Menu_obj;
Tab_Name Current::Tab = Tab_None;

void Current::none(plot_obj_type parent_type) {
  switch (parent_type) {
    case po_root:
      nl_error(-2, "No current figure");
      Current::Figure = NULL;
      switch (Current::Tab) {
        case Tab_Figure:
        case Tab_X:
        case Tab_Y:
        case Tab_Line:
          PtSetResource(ABW_ConsoleGroup, Pt_ARG_PG_CURRENT, "Graphs", 0);
          break;
        default:
          break;
      }
    case po_figure:
      nl_error(-2, "No current pane");
      Current::Pane = NULL;
      switch (Current::Tab) {
        case Tab_X:
        case Tab_Y:
        case Tab_Line:
          PtSetResource(ABW_ConsoleGroup, Pt_ARG_PG_CURRENT, "Graphs", 0);
          break;
        default:
          break;
      }
    case po_pane:
      nl_error(-2, "No current axes");
      Current::Axes = NULL;
      Current::Axis = NULL;
      switch (Current::Tab) {
        case Tab_X:
        case Tab_Y:
          plot_axis::Clear_Axis_Pane();
          break;
        case Tab_Line:
          PtSetResource(ABW_ConsoleGroup, Pt_ARG_PG_CURRENT, "Graphs", 0);
          break;
        default:
          break;
      }
    case po_axes:
      nl_error(-2, "No current graph");
      Current::Graph = NULL;
      if (Current::Tab == Tab_Line)
        PtSetResource(ABW_ConsoleGroup, Pt_ARG_PG_CURRENT, "Graphs", 0);
    case po_data:
      nl_error(-2, "No current line");
      Current::Line = NULL;
      /* Defer this test until we find out if there are lines */
      // if (Current::Tab == Tab_Line)
      //   PtSetResource(ABW_ConsoleGroup, Pt_ARG_PG_CURRENT, "Graphs", 0);
    default:
      break;
  }
}
