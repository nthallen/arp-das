/* figure.cc
  Support routines for the Figure module
  */
#include <ctype.h>
#include "ablibs.h"
#include "phrtg.h"
#include "abimport.h"
#include "nl_assert.h"

std::list<plot_figure*> All_Figures;

plot_figure::plot_figure( const char *name_in) : plot_obj(po_figure, name_in) {
  min_dim.h = min_dim.w = 2; // 2*Divider Bezel
  saw_first_resize = false;
  display_name = true;
  synch_x = true;
  ApModuleParent( ABM_Figure, AB_NO_PARENT, NULL );
  module = ApCreateModule( ABM_Figure, NULL, NULL );
  window = ApGetWidgetPtr(module, ABN_Figure);
  PtWidget_t *divider = ApGetWidgetPtr(module, ABN_Figure_Div);
  PtSetResource(window, Pt_ARG_POINTER, this, 0 );
  PtSetResource(window, Pt_ARG_WINDOW_TITLE, name, 0);
  PtSetResource(module, Pt_ARG_POINTER, this, 0 );
  PtSetResource(divider, Pt_ARG_POINTER, this, 0 );
  PhDim_t *div_dim;
  PtGetResource(divider, Pt_ARG_DIM, &div_dim, 0 );
  dim = *div_dim;
  if ( !All_Figures.empty() )
    PtTreeAddAfter(ABW_Graphs_Tab, TreeItem, All_Figures.back()->TreeItem);
  else PtTreeAddFirst(ABW_Graphs_Tab, TreeItem, NULL);
  All_Figures.push_back(this);
  nl_error(-2, "plot_figure %s created", name);
}

plot_figure::~plot_figure() {
  if ( destroying ) return;
    destroying = true;
  while (! panes.empty()) delete panes.front();
  PtSetResource(window, Pt_ARG_POINTER, NULL, 0 );
  PtSetResource(module, Pt_ARG_POINTER, NULL, 0 );
  PtWidget_t *divider = ApGetWidgetPtr(module, ABN_Figure_Div);
  PtSetResource(divider, Pt_ARG_POINTER, NULL, 0 );

  All_Figures.remove(this);
  nl_error(-3,"All_Figures now has %d elements", All_Figures.size());
  TreeItem->data = NULL;
  if (!(PtWidgetFlags(module) & Pt_DESTROYED))
    PtDestroyWidget(module);
  module = NULL;
  if (this == Current::Figure) {
    if (All_Figures.empty())Current::none(po_root);
    else All_Figures.front()->got_focus(focus_from_parent);
  }
}

/* plot_figure::AddChild(plot_pane *p);
 * Called from the child during construction
 * Responsible for adding the child to the end of
 * the list of children and adding it's TreeItem to 
 * the tree hierarchy
 */
void plot_figure::AddChild(plot_pane *p) {
  // Change_min_dim(0, p->min_height);
  if ( !panes.empty() ) {
    PtTreeAddAfter(ABW_Graphs_Tab, p->TreeItem, panes.back()->TreeItem);
  } else {
    PtTreeAddFirst(ABW_Graphs_Tab, p->TreeItem, TreeItem);
  }
  panes.push_back(p);
  Adjust_Panes(p->min_height);
  if (current_child == NULL) current_child = p;
}

/* void plot_figure::RemoveChild(plot_pane *p);
 * Called from the child during its destruction.
 * Responsible for removing the child from the
 * list of children and possibly removing and
 * freeing it's TreeItem from the tree hierarchy.
 * This last step will be skipped if the figure
 * is in the process of destruction itself, since
 * the entire subtree will be removed and freed
 * when the figure is destroyed.
 */
void plot_figure::RemoveChild(plot_pane *p) {
  nl_assert(p != NULL);
  nl_assert(!panes.empty());
  if (current_child == p) current_child = NULL;
  else {
    nl_assert(Current::Pane != p);
  }
  panes.remove(p);
  if (!destroying) {
  	PtDestroyWidget(p->widget);
    Adjust_Panes( -p->min_height );
    if (current_child == NULL) {
      current_child = default_child();
      if (Current::Pane == p) {
        if (current_child == NULL) Current::none(type);
        else current_child->got_focus(focus_from_parent);
      }
    }
  }
}

void plot_figure::rename(const char *text, Update_Source src) {
  plot_obj::rename(text, src);
  PtSetResource(window, Pt_ARG_WINDOW_TITLE, name, 0);
  if (src == from_file && Current::Figure == this
      && Current::Tab == Tab_Figure) {
    PtSetResource(ABW_Figure_Name, Pt_ARG_TEXT_STRING,
        Current::Figure->name, 0);
  }
}

void plot_figure::Adjust_Panes(int delta_min_height) {
  Change_min_dim( 0, delta_min_height);
  if (!destroying)
    resized(&dim, &dim, 1);
}

plot_pane *plot_figure::CreateGraph(RTG_Variable_Data *var) {
  nl_assert(var != NULL);
  plot_pane *pane = new plot_pane(var->name, this);
  pane->CreateGraph(var);
  return pane;
}

void plot_figure::got_focus(focus_source whence) {
  if (this == Current::Figure) return;
  plot_obj::got_focus(whence);
  Current::Figure = this;
  Update_Window_Tab();
}

void plot_figure::Update_Window_Tab() {
  // Update Window_Tab 
  PtSetResource(ABW_Figure_Name, Pt_ARG_TEXT_STRING, name, 0);
  PtSetResource(ABW_Figure_Visible, Pt_ARG_FLAGS,
      visible ? Pt_TRUE : Pt_FALSE, Pt_SET);
}

bool plot_figure::render() {
  std::list<plot_pane*>::const_iterator pos;
  for (pos = panes.begin(); pos != panes.end(); pos++) {
    plot_pane *p = *pos;
    if ( p->render() ) return true;
  }
  return false;
}

plot_obj *plot_figure::default_child() {
  if (panes.empty()) return NULL;
  else return panes.front();
}

bool plot_figure::check_for_updates() {
  bool updates_required = false;
  std::list<plot_pane*>::const_iterator pos;
  if (visible && !new_visibility) {
    // save current Pt_ARG_POS
    PhPoint_t *PosP;
    PhPoint_t OffScreen = { -32000, 0 };
    PtGetResource(window, Pt_ARG_POS, &PosP, 0);
    Pos = *PosP;
    PtSetResource(window, Pt_ARG_POS, &OffScreen, 0);
    PtSetResource(window, Pt_ARG_FLAGS, Pt_TRUE, Pt_BLOCKED );
    PtSetResource(window, Pt_ARG_WINDOW_MANAGED_FLAGS, Pt_FALSE,
        Ph_WM_FOCUS);
    nl_error(-2,"Hiding: old pos (%d,%d)", Pos.x, Pos.y);
  } else if (!visible && new_visibility) {
    PtSetResource(window, Pt_ARG_POS, &Pos, 0);
    PtSetResource(window, Pt_ARG_FLAGS, Pt_FALSE, Pt_BLOCKED );
    PtSetResource(window, Pt_ARG_WINDOW_MANAGED_FLAGS, Pt_TRUE,
        Ph_WM_FOCUS);
    nl_error(-2,"Restoring: old pos (%d,%d)", Pos.x, Pos.y);
  }
  visible = new_visibility;
  for (pos = panes.begin(); pos != panes.end(); pos++) {
    plot_pane *p = *pos;
    if ( p->check_for_updates(visible) )
      updates_required = true;
  }
  return updates_required;
}

/*
 * At this point plot_figure::Setup() is entirely informational.
 * It can probabably be disabled.
 */
int plot_figure::Setup( PtWidget_t *link_instance, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
	/* eliminate 'unreferenced' warnings */
	apinfo = apinfo, cbinfo = cbinfo;

  #ifdef DEBUG_WINDOW_SIZE
    PtWidget_t *divider = ApGetWidgetPtr(link_instance, ABN_Figure_Div);
    PtWidget_t *window = ApGetWidgetPtr(link_instance, ABN_Figure);
    nl_assert(divider != NULL && window != NULL);

    PhDim_t *win_dim, *div_dim;
    PtGetResource(window, Pt_ARG_DIM, &win_dim, 0 );
    nl_error(0, "window dimensions during setup (%d,%d)", win_dim->w, win_dim->h );
    PtGetResource(divider, Pt_ARG_DIM, &div_dim, 0 );
    nl_error(0, "Divider (%p) dimensions during setup (%d,%d)",
    		divider, div_dim->w, div_dim->h );
  #endif
  
	return( Pt_CONTINUE );
}

/* plot_figure::Realized() static member function
 * Realized callback function
 * This function is apparently superfluous
 */
int plot_figure::Realized( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
	/* eliminate 'unreferenced' warnings */
	apinfo = apinfo, cbinfo = cbinfo;
#ifdef VERBOSE
	PtWidget_t *module = ApGetInstance(widget);
	PtWidget_t *window = ApGetWidgetPtr(module, ABN_Figure);
    PtWidget_t *divider = ApGetWidgetPtr(module, ABN_Figure_Div);
    PhDim_t *win_dim, *div_dim;
    PtGetResource(window, Pt_ARG_DIM, &win_dim, 0 );
    nl_error(0, "Window dimensions on realize (%d,%d)", win_dim->w, win_dim->h );
    PtGetResource(divider, Pt_ARG_DIM, &div_dim, 0 );
    nl_error(0, "Divider (%p) dimensions on realize (%d,%d)",
    		divider, div_dim->w, div_dim->h );
    // At this point, the plot_figure object has not been
    // attached to the widget, so there is no more we can do.
#else
    widget = widget;
#endif
	return( Pt_CONTINUE );
}

/* plot_figure::Report() static member function
 * Just used for testing.
 */
void plot_figure::Report() {
  std::list<plot_figure*>::const_iterator pos;
  PtWidget_t *divider;
  PhDim_t *div_dim, *win_dim;
  for (pos = All_Figures.begin(); pos != All_Figures.end(); pos++) {
  	plot_figure *figure = *pos;
  	//window = ApGetWidgetPtr(figure->module, ABN_Figure);
  	PtGetResource(figure->window, Pt_ARG_DIM, &win_dim, 0);
  	nl_error( 0, "Report: Window (%p) dims (%d,%d)",
  			  figure->window, win_dim->w, win_dim->h);
  	divider = ApGetWidgetPtr(figure->module, ABN_Figure_Div);
  	PtGetResource(divider, Pt_ARG_DIM, &div_dim, 0);
  	nl_error( 0, "Report: Divider (%p) dims (%d,%d)",
  	  divider, div_dim->w, div_dim->h);
  	std::list<plot_pane*>::const_iterator pp;
  	unsigned int n_panes = 0;
  	for ( pp = figure->panes.begin(); pp != figure->panes.end(); ++pp ) {
  	  plot_pane *p = *pp;
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
  	nl_assert(n_panes == figure->panes.size());
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
  int rv = fig->resized(&cb->old_dim, &cb->new_dim, 0);
  plot_obj::render_one();
  return rv;
}

int plot_figure::resized(PhDim_t *old_dim, PhDim_t *new_dim, bool force) {
  int cum_height = 0, cum_min = 0, rem_height;
  PhDim_t pane_dim;

  if ( !eq_dims(&dim, old_dim)) {
  	nl_error(2,"Missed a resize somewhere");
  	saw_first_resize = true;
  }
  if (!force) {
    if ( eq_dims(&dim, new_dim)) {
  	  // No actual resize
  	  saw_first_resize = false;
  	  return Pt_CONTINUE;
    }
    if ( !saw_first_resize) {
  	  saw_first_resize = true;
  	  return Pt_CONTINUE;
    }
    nl_error(-2, "divider resized");
  }
  saw_first_resize = false;
  dim = *new_dim;
  //plot_figure::Report();
  pane_dim.w = dim.w - 2;
  if (panes.empty()) return Pt_CONTINUE;
  std::list<plot_pane*>::const_iterator pp;
  for ( pp = panes.begin(); pp != panes.end(); ++pp ) {
  	plot_pane *pane = *pp;
  	if (pane->visible) {
    	cum_height += pane->full_height;
    	cum_min += pane->min_height;
  	}
  }
  rem_height = dim.h - 2;
  if ( rem_height < cum_min ) {
	nl_error( 2, "divider_resized: divider height %d < cum_min %d",
		rem_height, cum_min );
    return Pt_CONTINUE;
  }
  if ( rem_height > cum_height ) {
    // Add to each pane's height
    for ( pp = panes.begin(); pp != panes.end(); ++pp ) {
  	  plot_pane *pane = *pp;
  	  if (pane->visible) {
    	  int new_height = (rem_height * pane->full_height)/cum_height;
    	  pane_dim.h = new_height;
    	  rem_height -= new_height;
    	  cum_height -= pane->full_height;
    	  PtSetResource( pane->widget, Pt_ARG_DIM, &pane_dim, 0);
    	  pane->resized(&pane_dim);
  	  }
  	}
  	if (cum_height != 0 || rem_height != 0)
  	  nl_error( 1, "divider_resized up: cum_height %d rem_height %d",
  			  cum_height, rem_height);
  } else if ( rem_height < cum_height ) {
  	// allocate the amount above minimum
  	rem_height -= cum_min;
  	cum_height -= cum_min;
  	nl_assert(cum_height > 0); // it's > rem_height >= cum_min
  	for ( pp = panes.begin(); pp != panes.end(); ++pp ) {
  	  plot_pane *pane = *pp;
  	  if (pane->visible) {
        int dh = pane->full_height - pane->min_height;
    	  int new_height = cum_height > 0 ? (rem_height * dh)/cum_height : 0;
    	  rem_height -= new_height;
    	  cum_height -= dh;
    	  pane_dim.h = new_height + pane->min_height;
    	  PtSetResource( pane->widget, Pt_ARG_DIM, &pane_dim, 0);
    	  pane->resized(&pane_dim);
  	  }
  	}
  	if (cum_height != 0 || rem_height != 0)
  	  nl_error( 1, "divider_resized down: cum_height %d rem_height %d",
  			  cum_height, rem_height);
  } else {
    for ( pp = panes.begin(); pp != panes.end(); ++pp ) {
  	  plot_pane *pane = *pp;
  	  if (pane->visible) {
    	  pane_dim.h = pane->full_height;
    	  pane->resized(&pane_dim);
  	  }
  	}
  }
  //plot_figure::Report();
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
	std::list<plot_pane*>::const_iterator pp;
  plot_pane *pane;
  int i;

  /* eliminate 'unreferenced' warnings */
	apinfo = apinfo;
	switch (cbinfo->reason_subtype) {
	  case Ph_EV_DRAG_COMPLETE:
  		// Need to reset pane sizes accordingly
  		nl_error(-2, "div_drag: COMPLETE: %d panes", cb->nsizes);
  		PtGetResource(widget, Pt_ARG_POINTER, &fig, 0);
  		PtGetResource(widget, Pt_ARG_DIM, &div_dim, 0);
  		pane_dim = *div_dim;
  		pane_dim.w -= 2;
	    for ( i = 0, pp = fig->panes.begin(); i < cb->nsizes; ++i, ++pp ) {
  		  nl_assert(pp != fig->panes.end());
  		  pane = *pp;
  		  if (pane->visible) {
    		  pane_dim.h = cb->sizes[i].to - cb->sizes[i].from + 1;
    		  nl_assert(pane_dim.h >= pane->min_height);
    		  pane->resized(&pane_dim);
  		  }
  		}
	    plot_obj::render_one();
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

int plot_figure::unrealized( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
  /* eliminate 'unreferenced' warnings */
  widget = widget, apinfo = apinfo, cbinfo = cbinfo;

  nl_error(-2, "plot_figure::unrealized");
  return( Pt_CONTINUE );
}


int plot_figure::destroyed( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
  plot_figure *fig;
  PtWidget_t *module;
  /* eliminate 'unreferenced' warnings */
  apinfo = apinfo, cbinfo = cbinfo;
  module = ApGetInstance(widget);
  PtGetResource(module, Pt_ARG_POINTER, &fig, 0);
  if ( fig == NULL ) {
    nl_error(-2, "plot_figure::destroyed with NULL data pointer");
  } else {
  	nl_error(-2, "plot_figure::destroyed %s", fig->name);
  	delete fig;
  }
  return( Pt_CONTINUE );
}


int plot_figure::wmevent( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
  const char *event, *state;
  /* eliminate 'unreferenced' warnings */
  widget = widget, apinfo = apinfo;
  PhWindowEvent_t *wevt = (PhWindowEvent_t *)cbinfo->cbdata;
  switch (wevt->event_f) {
  	case Ph_WM_CLOSE: event = "Ph_WM_CLOSE"; break;
  	case Ph_WM_FOCUS: return Pt_CONTINUE; //event = "Ph_WM_FOCUS"; break;
  	case Ph_WM_MENU: event = "Ph_WM_MENU"; break;
  	case Ph_WM_TOFRONT: event = "Ph_WM_TOFRONT"; break;
  	case Ph_WM_TOBACK: event = "Ph_WM_TOBACK"; break;
  	case Ph_WM_CONSWITCH: event = "Ph_WM_CONSWITCH"; break;
  	case Ph_WM_RESIZE: event = "Ph_WM_RESIZE"; break;
  	case Ph_WM_MOVE: event = "Ph_WM_MOVE"; break;
  	case Ph_WM_HIDE: event = "Ph_WM_HIDE"; break;
  	case Ph_WM_MAX: event = "Ph_WM_MAX"; break;
  	case Ph_WM_BACKDROP: event = "Ph_WM_BACKDROP"; break;
  	case Ph_WM_RESTORE: event = "Ph_WM_RESTORE"; break;
  	case Ph_WM_HELP: event = "Ph_WM_HELP"; break;
  	case Ph_WM_FFRONT: event = "Ph_WM_FFRONT"; break;
  	default: event = "unknown"; break;
  }
  switch (wevt->state_f) {
    case Ph_WM_STATE_ISNORMAL: state = "Ph_WM_STATE_ISNORMAL"; break;
    case Ph_WM_STATE_ISHIDDEN: state = "Ph_WM_STATE_ISHIDDEN"; break;
    case Ph_WM_STATE_ISMAX: state = "Ph_WM_STATE_ISMAX"; break;
    case Ph_WM_STATE_ISICONIFIED: state = "Ph_WM_STATE_ISICONIFIED"; break;
    case Ph_WM_STATE_ISTASKBAR: state = "Ph_WM_STATE_ISTASKBAR"; break;
    case Ph_WM_STATE_ISBACKDROP: state = "Ph_WM_STATE_ISBACKDROP"; break;
    default: state = "unknown"; break;
  }
  nl_error(-2, "plot_figure::wmevent %s state %s", event, state);
  return( Pt_CONTINUE );
}
