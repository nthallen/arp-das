/* figure.cc
  Support routines for the Figure module
  */
#include <ctype.h>
#include "ablibs.h"
#include "phrtg.h"
#include "abimport.h"
#include "nl_assert.h"

plot_figure *Cur_Figure, *All_Figures;

plot_obj::plot_obj(plot_obj_type po_type, const char *name_in) {
  type = po_type;
  TreeItem = NULL;
  if (name_in == NULL) name_in = typetext();
  name = strdup(name_in);
}

const char *plot_obj::typetext() {
  switch ( type ) {
    case po_figure: return "figure";
    case po_pane: return "pane";
    case po_axes: return "axes";
    case po_data: return "data";
    case po_line: return "line";
    case po_text: return "text";
    default: return "unknown";
  }
}

void plot_obj::TreeAllocItem() {
  char temp_buf[80];
  if ( snprintf(temp_buf, 80, "%s\t%s", name, typetext()) >= 80 )
	nl_error(2,"Variable name exceeds buffer length in TreeAllocItem");
  TreeItem = PtTreeAllocItem(ABW_Graphs_Tab, temp_buf, -1, -1);
  TreeItem->data = (void *)this;
}



plot_figure::plot_figure( const char *name_in) : plot_obj(po_figure, name_in) {
  ApModuleParent( ABM_Figure, AB_NO_PARENT, NULL );
  module = ApCreateModule( ABM_Figure, NULL, NULL );
  window = ApGetWidgetPtr(module, ABN_Figure);
  PtWidget_t *divider = ApGetWidgetPtr(module, ABN_Figure_Div);
  PtWidget_t *first_pane = ApGetWidgetPtr(module, ABN_Figure_Pane );
  PtSetResource(window, Pt_ARG_POINTER, this, 0 );
  PtSetResource(window, Pt_ARG_WINDOW_TITLE, name, 0);
  PtSetResource(module, Pt_ARG_POINTER, this, 0 );
  PtSetResource(divider, Pt_ARG_POINTER, this, 0 );
  PhDim_t *div_dim;
  PtGetResource(divider, Pt_ARG_DIM, &div_dim, 0 );
  dim = *div_dim;
  min_dim.h = min_dim.w = 2; // 2*Divider Bezel
  saw_first_resize = false;
  TreeAllocItem();
  if ( All_Figures != NULL )
	PtTreeAddAfter(ABW_Graphs_Tab, this->TreeItem, All_Figures->TreeItem);
  else PtTreeAddFirst(ABW_Graphs_Tab, this->TreeItem, NULL);
  new plot_pane(name, this, first_pane);
  Cur_Figure = this;
  this->next = All_Figures;
  All_Figures = this;
  nl_error(0, "plot_figure %s created", name);
}

/*
 * At this point plot_figure::Setup() is entirely informational.
 * It can probabably be disabled.
 */
int plot_figure::Setup( PtWidget_t *link_instance, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
	/* eliminate 'unreferenced' warnings */
	link_instance = link_instance, apinfo = apinfo, cbinfo = cbinfo;
    PtWidget_t *divider = ApGetWidgetPtr(link_instance, ABN_Figure_Div);
    PtWidget_t *first_pane = ApGetWidgetPtr(link_instance, ABN_Figure_Pane );
    PtWidget_t *window = ApGetWidgetPtr(link_instance, ABN_Figure);
    nl_assert(divider != NULL && first_pane != NULL && window != NULL);

    PhDim_t *win_dim, *div_dim, *p1_dim;
    PtGetResource(window, Pt_ARG_DIM, &win_dim, 0 );
    nl_error(0, "window dimensions during setup (%d,%d)", win_dim->w, win_dim->h );
    PtGetResource(divider, Pt_ARG_DIM, &div_dim, 0 );
    nl_error(0, "Divider (%p) dimensions during setup (%d,%d)",
    		divider, div_dim->w, div_dim->h );
    PtGetResource(first_pane, Pt_ARG_DIM, &p1_dim, 0 );
    nl_error(0, "first pane dimensions during setup (%d,%d)", p1_dim->w, p1_dim->h );

	return( Pt_CONTINUE );
}

/* plot_figure::Realized() static member function
 * Realized callback function
 */
int plot_figure::Realized( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
	/* eliminate 'unreferenced' warnings */
	widget = widget, apinfo = apinfo, cbinfo = cbinfo;
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
      for (plot_pane *p = fig->first; p; p = p->next) {
    	PhDim_t *p_dim;
        PtGetResource(p->widget, Pt_ARG_DIM, &p_dim, 0 );
        nl_error(0, "pane %d dimensions on realize (%d,%d)",
        		pane_num++, p_dim->w, p_dim->h );
      }
    } else nl_error( 0, "module realized before figure attached");
	return( Pt_CONTINUE );
}

/* plot_figure::Report() static member function
 * Just used for testing.
 */
void plot_figure::Report() {
  plot_figure *figure;
  PtWidget_t *divider;
  PhDim_t *div_dim, *win_dim;
  int n_panes = 0;
  for (figure = All_Figures; figure != NULL; figure = (plot_figure *)figure->next) {
	//window = ApGetWidgetPtr(figure->module, ABN_Figure);
	PtGetResource(figure->window, Pt_ARG_DIM, &win_dim, 0);
	nl_error( 0, "Report: Window (%p) dims (%d,%d)",
			  figure->window, win_dim->w, win_dim->h);
	divider = ApGetWidgetPtr(figure->module, ABN_Figure_Div);
	PtGetResource(divider, Pt_ARG_DIM, &div_dim, 0);
	nl_error( 0, "Report: Divider (%p) dims (%d,%d)",
	  divider, div_dim->w, div_dim->h);
	for ( plot_pane *p = figure->first; p != NULL; p = p->next ) {
	  ++n_panes;
	  if ( p->widget != NULL) {
		PhDim_t *old_dim;
		PtGetResource(p->widget, Pt_ARG_DIM, &old_dim, 0);
		nl_error( 0, "Report: Pane %d (%p) dims (%d,%d)",
				n_panes, p->widget, old_dim->w, old_dim->h);
		if (old_dim->h != p->full_height)
			nl_error(1,"Report: Pane %d full_height %d dim %d",
			  n_panes, p->full_height, old_dim->h);
		if ( p->full_height < p->min_height )
		  nl_error( 1, "Report: Pane %d full_height %d min_height %d",
		    n_panes, p->full_height, p->min_height );
	  }
	}
  }
}

static inline bool eq_dims(PhDim_t *a, PhDim_t *b) {
  return a->w == b->w && a->h == b->h;
}

/*
 * plot_figure::divider_resized() static member function
 * This routine gets called twice, once before the panes
 * have been resized and once after. There is no clue
 * as to which pass is happening, so I need to keep
 * track.
 * 
 * If we add up the pane full_heights and min_heights, we
 * get cum_height and cum_min. The divider height should
 * never be less than cum_min+2. If the divider height is
 * equal to cum_height, we don't have to do anything
 * regarding pane height, though we may need to relay
 * a new width to the panes and on to the axes.
 * If the divider height is greater than cum_height,
 * we can to add height
 * to each pane in proportion to it's full_height.
 * If divider height is less than cum_height+2, we want
 * to allocate div_height-cum_min above each pane's
 * min_height in proportion to it's full_height-min_height 
 */
int plot_figure::divider_resized( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
  PtContainerCallback_t *cb = (PtContainerCallback_t *)cbinfo->cbdata;
  /* eliminate 'unreferenced' warnings */
  apinfo = apinfo;
  plot_figure *fig;
  PtGetResource(widget, Pt_ARG_POINTER, &fig, 0);
  return fig->Resized(cb);
}

int plot_figure::Resized(PtContainerCallback_t *cb) {
  int cum_height = 0, cum_min = 0, rem_height;
  PhDim_t pane_dim;

  if ( !eq_dims(&dim, &cb->old_dim)) {
	nl_error(2,"Missed a resize somewhere");
	saw_first_resize = true;
  }
  if ( eq_dims(&dim, &cb->new_dim)) {
	// No actual resize
	saw_first_resize = false;
	return Pt_CONTINUE;
  }
  if ( !saw_first_resize) {
	saw_first_resize = true;
	return Pt_CONTINUE;
  }
  nl_error(0, "divider resized");
  saw_first_resize = false;
  dim = cb->new_dim;
  plot_figure::Report();
  pane_dim.w = dim.w - 2;
  for ( plot_pane *pane = first; pane != NULL; pane = pane->next ) {
	cum_height += pane->full_height;
	cum_min += pane->min_height;
  }
  rem_height = dim.h - 2;
  if ( rem_height < cum_min ) {
	nl_error( 2, "divider_resized: divider height %d < cum_min %d",
		rem_height, cum_min );
    return Pt_CONTINUE;
  }
  if ( rem_height > cum_height ) {
	// Add to each pane's height
	for ( plot_pane *pane = first; pane != NULL; pane = pane->next ) {
	  int new_height = (rem_height * pane->full_height)/cum_height;
	  pane_dim.h = new_height;
	  rem_height -= new_height;
	  cum_height -= pane->full_height;
	  PtSetResource( pane->widget, Pt_ARG_DIM, &pane_dim, 0);
	  pane->resized(&pane_dim);
	}
	if (cum_height != 0 || rem_height != 0)
	  nl_error( 1, "divider_resized up: cum_height %d rem_height %d",
			  cum_height, rem_height);
  } else if ( rem_height < cum_height ) {
	// allocate the amount above minimum
	rem_height -= cum_min;
	cum_height -= cum_min;
	nl_assert(cum_height > 0); // it's > rem_height >= cum_min
	for ( plot_pane *pane = first; pane != NULL; pane = pane->next ) {
      int dh = pane->full_height - pane->min_height;
	  int new_height = cum_height > 0 ? (rem_height * dh)/cum_height : 0;
	  rem_height -= new_height;
	  cum_height -= dh;
	  pane_dim.h = new_height + pane->min_height;
	  PtSetResource( pane->widget, Pt_ARG_DIM, &pane_dim, 0);
	  pane->resized(&pane_dim);
	}
	if (cum_height != 0 || rem_height != 0)
	  nl_error( 1, "divider_resized down: cum_height %d rem_height %d",
			  cum_height, rem_height);
  } else {
	for ( plot_pane *pane = first; pane != NULL; pane = pane->next ) {
	  pane_dim.h = pane->full_height;
	  pane->resized(&pane_dim);
	} 
  }
  plot_figure::Report();
  return( Pt_CONTINUE );
}

void plot_figure::Change_min_dim(int dw, int dh) {
  PhDim_t real_min;
  if (dw<0 && min_dim.w+DIV_BEVEL_WIDTHS < -dw) {
	nl_error(2, "figure min.w (%d) < 0", min_dim.w );
	dw = DIV_BEVEL_WIDTHS-min_dim.w;
  }
  if (dh<0 && min_dim.h+DIV_BEVEL_WIDTHS < -dh) {
	nl_error(2, "figure min.h (%d) < 0", min_dim.h );
	dh = DIV_BEVEL_WIDTHS-min_dim.h;
  }
  min_dim.w += dw;
  min_dim.h += dh;
  real_min.w = min_dim.w > 40 ? min_dim.w : 40;
  real_min.h = min_dim.h > 20 ? min_dim.h : 20;
  PtSetResource(window,Pt_ARG_MINIMUM_DIM, &real_min, 0);
}

/*
 * plot_figure::divider_drag() static member function
 * gets called while the divider handle is being dragged
 * The reason_subtypes we expect to see are MOVE, BOUNDARY and COMPLETE
 * The only one we really care about is COMPLETE, which must notify
 * the panes of their new sizes and make sure the graphs are redrawn.
 * We may want to do something on MOVE (or the same thing)
 * BOUNDARY we don't care about. The other types are unexpected.
 * 
 * There is another callback: Pt_DIVIDER_HANDLE_CALLBACK, which
 * offers the opportunity to decline the drag operation.
 */
int plot_figure::divider_drag( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
    PtDividerCallback_t *cb = (PtDividerCallback_t *)cbinfo->cbdata;
    PhDim_t *div_dim, pane_dim;
    plot_figure *fig;
    plot_pane *pane;
    int i;

    /* eliminate 'unreferenced' warnings */
	widget = widget, apinfo = apinfo, cbinfo = cbinfo;
	switch (cbinfo->reason_subtype) {
	  case Ph_EV_DRAG_COMPLETE:
		// Need to reset pane sizes accordingly
		nl_error(0, "div_drag: COMPLETE: %d panes", cb->nsizes);
		PtGetResource(widget, Pt_ARG_POINTER, &fig, 0);
		PtGetResource(widget, Pt_ARG_DIM, &div_dim, 0);
		pane_dim = *div_dim;
		for ( i = 0, pane = fig->first; i < cb->nsizes; pane = pane->next, ++i ) {
		  nl_assert(pane != NULL);
		  pane_dim.h = cb->sizes[i].to - cb->sizes[i].from + 1;
		  nl_assert(pane_dim.h >= pane->min_height);
		  pane->resized(&pane_dim);
		}
		break;
	  case Ph_EV_DRAG_MOVE:
		break; // expected and may be interesting
	  case Ph_EV_DRAG_BOUNDARY:
		break; // expected but uninteresting
	  case Ph_EV_DRAG_INIT:
	  case Ph_EV_DRAG_KEY_EVENT:
	  case Ph_EV_DRAG_MOTION_EVENT:
	  case Ph_EV_DRAG_START:
	  default:
		nl_error(1, "div_drag: Unexpected event subtype: %d",
				cbinfo->reason_subtype);
		break;
	}
	return( Pt_CONTINUE );
}

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
  min_height = min_dim.h;
  next = NULL;
  widget = pane;
  parent->Change_min_dim(0, min_height);
  TreeAllocItem();
  if (parent->last != NULL) {
	PtTreeAddAfter(ABW_Graphs_Tab, this->TreeItem, parent->last->TreeItem);
	parent->last->next = this;
  } else {
	PtTreeAddFirst(ABW_Graphs_Tab, this->TreeItem, parent->TreeItem);
	parent->first = this;
  }
  parent->last = this;
  
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
  for ( plot_pane *p = figure->first; p != NULL; p = p->next ) {
	++n_panes;
	if ( p->widget != NULL) {
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
  if (cum_height != rem_height)
	nl_error( 1, "Old pane heights are %d, not %d", cum_height, rem_height);
  if (pane == NULL) {
	full_height = cum_height/n_panes;
	cum_height += full_height;
	PtArg_t args[1];
	pane_dim.h = full_height;
	PtSetArg( &args[0], Pt_ARG_ANCHOR_FLAGS, 0, Pt_TRUE);
	widget = PtCreateWidget(PtPane, divider, 1, args );
	PtRealizeWidget(widget);
  }

  n_panes = 0;
  // Go through the all panes and adjust their size
  for ( plot_pane *p = figure->first; p != NULL; p = p->next ) {
    ++n_panes;
	if (p->widget != NULL) {
	  pane_dim.h = (rem_height * p->full_height)/cum_height;
	  PtSetResource(p->widget, Pt_ARG_DIM, &pane_dim, 0);
      cum_height -= p->full_height;
	  p->full_height = pane_dim.h;
	  rem_height -= pane_dim.h;
	  nl_error(0, "Setting pane %d (%p) to (%d,%d)", n_panes, p->widget, pane_dim.w, pane_dim.h);
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

void plot_pane::resized(PhDim_t *newdim) {
  full_height = newdim->h;
  nl_assert( full_height >= min_height );
  // Then we relay this information to the axes
}

int plot_obj::TreeSelected( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
  /* eliminate 'unreferenced' warnings */
  widget = widget, apinfo = apinfo;
  if (cbinfo->reason_subtype == Pt_LIST_SELECTION_FINAL) {
	PtTreeCallback_t *cb = (PtTreeCallback_t *)cbinfo->cbdata;
	plot_obj *p = (plot_obj *)cb->item->data;
    nl_error( 0, "Graphs TreeSelected (Final) %s:%s", p->name, p->typetext());
  }
  return( Pt_CONTINUE );
}


int plot_obj::ToggleVisible( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
  /* eliminate 'unreferenced' warnings */
  widget = widget, apinfo = apinfo, cbinfo = cbinfo;
  return( Pt_CONTINUE );
}

int plot_obj::Delete( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
  /* eliminate 'unreferenced' warnings */
  widget = widget, apinfo = apinfo, cbinfo = cbinfo;
  return( Pt_CONTINUE );
}

int plot_obj::context_menu_setup( PtWidget_t *link_instance,
		ApInfo_t *apinfo, PtCallbackInfo_t *cbinfo ) {
  /* eliminate 'unreferenced' warnings */
  link_instance = link_instance, apinfo = apinfo, cbinfo = cbinfo;
  return( Pt_CONTINUE );
}

int plot_obj::TreeColSelect( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
  /* eliminate 'unreferenced' warnings */
  PtTreeCallback_t *cb = (PtTreeCallback_t *)cbinfo->cbdata;
  widget = widget, apinfo = apinfo;
  nl_error(0,"Graphs TreeColSelect: col %d, nitems %d", cb->column, cb->nitems);
  return( Pt_CONTINUE );
}


int plot_obj::TreeInput( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
	/* eliminate 'unreferenced' warnings */
	widget = widget, apinfo = apinfo, cbinfo = cbinfo;
	if (cbinfo->reason_subtype == Ph_EV_BUT_PRESS) {
	  PhPointerEvent_t *pe = (PhPointerEvent_t *)PhGetData(cbinfo->event);
	  if ( pe->buttons == Ph_BUTTON_MENU ) {
		PtGenTreeInput_t *ti = (PtGenTreeInput_t *)cbinfo->cbdata;
		PtTreeItem_t *item = (PtTreeItem_t *)ti->item;
		plot_obj *po = (plot_obj *)item->data;
		nl_error( 0, "plot_obj: Menu: %s:%s", po->name, po->typetext());
	  }
	}
	return( Pt_CONTINUE );
}

