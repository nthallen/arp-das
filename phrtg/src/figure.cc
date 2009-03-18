/* figure.cc
  Support routines for the Figure module
  */
#include <ctype.h>
#include "ablibs.h"
#include "phrtg.h"
#include "abimport.h"
#include "nl_assert.h"

plot_obj plot_obj::root(po_root);
plot_figure *Cur_Figure, *All_Figures;

plot_obj::plot_obj(plot_obj_type po_type, plot_obj *parent_in) {
  type = po_type;
  first = last = next = NULL;
  if ( parent_in == NULL && po_type != po_root )
	parent_in = &root;
  parent = parent_in;
  if ( parent != NULL ) {
	if (parent->last != NULL) {
	  parent->last->next = this;
	} else {
	  parent->first = this;
	}
    parent->last = this;
  }
}

plot_figure::plot_figure( const char *name_in) : plot_obj(po_figure) {
  if (name_in == NULL) name_in = "undefined";
  name = strdup(name_in);
  ApModuleParent( ABM_Figure, AB_NO_PARENT, NULL );
  module = ApCreateModule( ABM_Figure, NULL, NULL );
  PtWidget_t *window = ApGetWidgetPtr(module, ABN_Figure);
  PtWidget_t *first_pane = ApGetWidgetPtr(module, ABN_Figure_Pane );
  total_weight = 0;
  PtSetResource(window, Pt_ARG_POINTER, this, 0 );
  PtSetResource(window, Pt_ARG_WINDOW_TITLE, name, 0);
  new plot_pane(name, this, first_pane);
  Cur_Figure = this;
  this->next = All_Figures;
  All_Figures = this;
  nl_error(0, "plot_figure %s created", name);
}

int
plot_figure::Setup( PtWidget_t *link_instance, ApInfo_t *apinfo, PtCallbackInfo_t *cbinfo )

	{

	/* eliminate 'unreferenced' warnings */
	link_instance = link_instance, apinfo = apinfo, cbinfo = cbinfo;
    PtWidget_t *divider = ApGetWidgetPtr(link_instance, ABN_Figure_Div);
    PtWidget_t *first_pane = ApGetWidgetPtr(link_instance, ABN_Figure_Pane );
    nl_assert(divider != NULL && first_pane != NULL);
    //PtSetResource(first_pane, Pt_ARG_FILL_COLOR, Pg_RED, 0 ); // Pg_GREEN


    PtWidget_t *window = ApGetWidgetPtr(link_instance, ABN_Figure);

    PhDim_t *win_dim, *div_dim, *p1_dim;
    PtGetResource(window, Pt_ARG_DIM, &win_dim, 0 );
    nl_error(0, "window dimensions during setup (%d,%d)", win_dim->w, win_dim->h );
    PtGetResource(divider, Pt_ARG_DIM, &div_dim, 0 );
    nl_error(0, "Divider (%p) dimensions during setup (%d,%d)",
    		divider, div_dim->w, div_dim->h );
    PtGetResource(first_pane, Pt_ARG_DIM, &p1_dim, 0 );
    nl_error(0, "first pane dimensions during setup (%d,%d)", p1_dim->w, p1_dim->h );
    //PhDim_t new_p1_dim;
    //new_p1_dim = *win_dim;
    //new_p1_dim.h = new_p1_dim.h > 30 ? new_p1_dim.h - 20 : new_p1_dim.h/2;
    //PtSetResource(first_pane, Pt_ARG_DIM, &new_p1_dim, 0);

    // Don't need to set the divider size as long as sum of
    // Pane sizes are smaller than the window
    //PtSetResource(divider, Pt_ARG_DIM, win_dim, 0);

	return( Pt_CONTINUE );
	}


int
plot_figure::Realized( PtWidget_t *widget, ApInfo_t *apinfo, PtCallbackInfo_t *cbinfo )
	{

	/* eliminate 'unreferenced' warnings */
	widget = widget, apinfo = apinfo, cbinfo = cbinfo;
   // Now we'd better reset the divider's size
	PtWidget_t *module = ApGetInstance(widget);
	PtWidget_t *window = ApGetWidgetPtr(module, ABN_Figure);
    PtWidget_t *divider = ApGetWidgetPtr(module, ABN_Figure_Div);
    PhDim_t *win_dim, *div_dim;
    PtGetResource(window, Pt_ARG_DIM, &win_dim, 0 );
    nl_error(0, "Window dimensions on realize (%d,%d)", win_dim->w, win_dim->h );
    PtGetResource(divider, Pt_ARG_DIM, &div_dim, 0 );
    nl_error(0, "Divider (%p) dimensions on realize (%d,%d)",
    		divider, div_dim->w, div_dim->h );
    plot_figure *fig;
    PtGetResource(window, Pt_ARG_POINTER, &fig, 0);
    if ( fig != NULL ) {
      int pane_num = 1;
      for (plot_pane *p = (plot_pane*)fig->first; p; p = (plot_pane*)p->next) {
    	PhDim_t *p_dim;
        PtGetResource(p->widget, Pt_ARG_DIM, &p_dim, 0 );
        nl_error(0, "pane %d dimensions on realize (%d,%d)",
        		pane_num++, p_dim->w, p_dim->h );
      }
    } else nl_error( 0, "module realized before figure attached");
	return( Pt_CONTINUE );

	}

void plot_figure::Report() {
  plot_figure *figure;
  PtWidget_t *divider, *window;
  PhDim_t *div_dim, *win_dim;
  int n_panes = 0;
  for (figure = All_Figures; figure != NULL; figure = (plot_figure *)figure->next) {
	window = ApGetWidgetPtr(figure->module, ABN_Figure);
	PtGetResource(window, Pt_ARG_DIM, &win_dim, 0);
	nl_error( 0, "Report: Window (%p) dims (%d,%d)",
			  window, win_dim->w, win_dim->h);
	divider = ApGetWidgetPtr(figure->module, ABN_Figure_Div);
	PtGetResource(divider, Pt_ARG_DIM, &div_dim, 0);
	nl_error( 0, "Report: Divider (%p) dims (%d,%d)",
	  divider, div_dim->w, div_dim->h);
	for ( plot_pane *p = (plot_pane *)figure->first; p != NULL;
	  			p = (plot_pane *)p->next ) {
	  ++n_panes;
	  if ( p->widget != NULL) {
		PhDim_t *old_dim;
		PtGetResource(p->widget, Pt_ARG_DIM, &old_dim, 0);
		nl_error( 0, "Report: Pane %d (%p) dims (%d,%d)",
				n_panes, p->widget, old_dim->w, old_dim->h);
	  }
	}
  }
}

plot_pane::plot_pane( const char *name_in, plot_figure *figure,
		PtWidget_t *pane) : plot_obj(po_pane, figure) {
  int n_panes = 0;
  PtWidget_t *divider, *window;
  PhDim_t *div_dim, *win_dim;
  PhDim_t pane_dim, min_dim = {20,20};
  int rem_height, rem_weight;
  
  if (name_in == NULL) name_in = "pane";
  name = strdup(name_in);
  widget = pane;
  weight = 100;
  figure->total_weight += weight;

  window = ApGetWidgetPtr(figure->module, ABN_Figure);
  PtGetResource(window, Pt_ARG_DIM, &win_dim, 0);
  nl_error( 0, "Window (%p) dims for new pane (%d,%d)",
		  window, win_dim->w, win_dim->h);
  divider = ApGetWidgetPtr(figure->module, ABN_Figure_Div);
  PtGetResource(divider, Pt_ARG_DIM, &div_dim, 0);
  nl_error( 0, "Divider (%p) dims for new pane (%d,%d)",
		  divider, div_dim->w, div_dim->h);
  rem_height = div_dim->h - 2; // Actually less the bevel*2
  rem_weight = figure->total_weight;
  pane_dim.w = div_dim->w - 2;

  // Go through the preexisting panes and report their size
  for ( plot_pane *p = (plot_pane *)figure->first; p != NULL;
  			p = (plot_pane *)p->next ) {
	++n_panes;
	if ( p->widget != NULL) {
	  PhDim_t *old_dim;
	  unsigned *aflags;
	  PtGetResource(p->widget, Pt_ARG_DIM, &old_dim, 0);
	  PtGetResource(p->widget, Pt_ARG_ANCHOR_FLAGS, &aflags, 0 );
	  nl_error( 0, "Pane %d (%p) aflags (%x) dims (%d,%d)",
			n_panes, p->widget, *aflags, old_dim->w, old_dim->h);
	}
  }
  if (pane == NULL) {
	PtArg_t args[2];
	pane_dim.h = 0;
	PtSetArg( &args[0], Pt_ARG_ANCHOR_FLAGS, 0, Pt_TRUE);
	PtSetArg( &args[1], Pt_ARG_DIM, &pane_dim, 0);
	widget = PtCreateWidget(PtPane, divider, 2, args );
	PtRealizeWidget(widget);
	nl_error( 0, "Divider (%p) now (%d,%d)", divider, div_dim->w, div_dim->h);
	PhRect_t offsets = {{0,0},{0,0}};
	PtSetResource(divider, Pt_ARG_ANCHOR_OFFSETS, &offsets, 0);
	nl_error( 0, "Divider (%p) after offsets (%d,%d)", divider, div_dim->w, div_dim->h);
  }

  n_panes = 0;
  
  // Go through the all panes and adjust their size
  for ( plot_pane *p = (plot_pane *)figure->first; p != NULL;
  			p = (plot_pane *)p->next ) {
	++n_panes;
	if (p->widget != NULL) {
	  pane_dim.h = p->next ? (rem_height * weight)/rem_weight : (rem_height - 2);
	  PtSetResource(p->widget, Pt_ARG_DIM, &pane_dim, 0);
	  rem_height -= pane_dim.h;
	  rem_weight -= weight;
	  nl_error(0, "Setting pane %d (%p) to (%d,%d)", n_panes, p->widget, pane_dim.w, pane_dim.h);
	}
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
