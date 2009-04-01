/* pane.cc
  Support routines for the plot_pane module
  */
#include <ctype.h>
#include "ablibs.h"
#include "phrtg.h"
#include "abimport.h"
#include "nl_assert.h"

/*
 * This is the constructor for the plot_pane object. It can
 * be passed an existing PtPane widget (for the first pane
 * that exists when the window is created) or it can create
 * a new one.
 */
plot_pane::plot_pane( const char *name_in, plot_figure *figure,
		PtWidget_t *pane) : plot_obj(po_pane, name_in) {
  int n_panes = 0;
  PtWidget_t *divider, *window;
  PhDim_t *div_dimp, *win_dim;
  PhDim_t pane_dim, min_dim = {20,20};
  int rem_height, cum_height = 0;
  PhDim_t div_dim;
  
  parent = figure;
  parent_obj = figure;
  min_height = min_dim.h;
  widget = pane;
  synch_x = true;
  parent->AddChild(this);
  
  window = ApGetWidgetPtr(figure->module, ABN_Figure);
  PtGetResource(window, Pt_ARG_DIM, &win_dim, 0);
  nl_error( 0, "Window (%p) dims for new pane (%d,%d)",
		  window, win_dim->w, win_dim->h);
  divider = ApGetWidgetPtr(figure->module, ABN_Figure_Div);
  PtGetResource(divider, Pt_ARG_DIM, &div_dimp, 0);
  nl_error( 0, "Divider (%p) dims for new pane (%d,%d)",
		  divider, div_dimp->w, div_dimp->h);
  div_dim = *div_dimp;
  rem_height = div_dim.h - 2; // Actually less the bevel*2
  pane_dim.w = div_dim.w - 2;
  full_height = (pane == NULL) ? 0 : rem_height;

  // Go through the preexisting panes and report their size
  std::list<plot_pane*>::const_iterator pp;
  for (pp = figure->panes.begin(); pp != figure->panes.end(); ++pp ) {
	plot_pane *p = *pp;
	if ( p->widget != NULL) {
	  ++n_panes;
	  // This section should be just a check except for
	  // calculating cum_height. We should be able to
	  // skip PtGetResource if we've done things right.
	  PhDim_t *old_dim;
	  PtGetResource(p->widget, Pt_ARG_DIM, &old_dim, 0);
	  nl_error( 0, "Pane %d (%p) dims (%d,%d)",
			n_panes, p->widget, old_dim->w, old_dim->h);
	  if ( p->full_height != old_dim->h ) {
		nl_error( 1, "Pane %d full_height %d, old_dim %d",
				  n_panes, p->full_height, old_dim->h);
		p->full_height = old_dim->h;
	  }
	  cum_height += p->full_height;
	}
  }
  if (n_panes > 0 && cum_height != rem_height)
	nl_error( 1, "Old pane heights are %d, not %d", cum_height, rem_height);
  if (pane == NULL) {
	full_height = n_panes ? cum_height/(n_panes+1) : rem_height;
	cum_height += full_height;
	PtArg_t args[2];
	pane_dim.h = full_height;
	PtSetArg( &args[0], Pt_ARG_ANCHOR_FLAGS, Pt_FALSE, Pt_TRUE);
	PtSetArg( &args[1], Pt_ARG_FLAGS, Pt_TRUE,
			Pt_HIGHLIGHTED|Pt_GETS_FOCUS);
	widget = PtCreateWidget(PtPane, divider, 2, args );
	PtAddCallback(widget,Pt_CB_GOT_FOCUS,(PtCallbackF_t *)plot_obj::pt_got_focus,NULL);
	PtRealizeWidget(widget);
  }
  // PtAddCallback(widget, Pt_CB_RESIZE, plot_pane::resize_cb, this);
  
  n_panes = 0;
  // Go through all panes and adjust their size
  for (pp = figure->panes.begin(); pp != figure->panes.end(); ++pp ) {
	plot_pane *p = *pp;
    ++n_panes;
	if (p->widget != NULL) {
	  pane_dim.h = (rem_height * p->full_height)/cum_height;
	  PtSetResource(p->widget, Pt_ARG_DIM, &pane_dim, 0);
      cum_height -= p->full_height;
	  rem_height -= pane_dim.h;
	  nl_error(0, "Setting pane %d (%p) to (%d,%d)", n_panes, p->widget, pane_dim.w, pane_dim.h);
	  p->resized(&pane_dim);
	} else nl_error(2, "Widget was NULL");
  }
  if (n_panes&1) {
	  PtSetResource(widget, Pt_ARG_FILL_COLOR, Pg_RED, 0 );
	  nl_error(0, "Setting color to Pg_RED");
  } else {
	  PtSetResource(widget, Pt_ARG_FILL_COLOR, Pg_GREEN, 0 );
	  nl_error(0, "Setting color to Pg_GREEN");
  }
  PtSetResource(widget, Pt_ARG_POINTER, this, 0 );
  PtSetResource(widget, Pt_ARG_MINIMUM_DIM, &min_dim, 0 );
}

plot_pane::~plot_pane() {
  if ( destroying ) return;
  destroying = true;
  if (this == Current::Pane)
	Current::Pane = NULL;
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
  axes.remove(ax);
}

void plot_pane::CreateGraph(RTG_Variable *var) {
  nl_assert(var != NULL);
  plot_axes *ax = new plot_axes(var->name, this);
  ax->CreateGraph(var);
}

void plot_pane::resized(PhDim_t *newdim ) {
  full_height = newdim->h;
  nl_assert( full_height >= min_height );
  nl_error(0,"Pane %s resized to (%d,%d)", name, newdim->w, newdim->h );
  // Then we relay this information to the axes
}

void plot_pane::got_focus(focus_source whence) {
  if (this == Current::Pane) return;
  plot_obj::got_focus(whence);
  Current::Pane = this;
  // Update dialogs for pane
}
