/* plot_obj.cc
  Support routines for the plot_obj virtual base class
  */
#include <ctype.h>
#include "ablibs.h"
#include "phrtg.h"
#include "abimport.h"
#include "nl_assert.h"

plot_obj *Current::Menu_obj;

plot_obj::plot_obj(plot_obj_type po_type, const char *name_in) {
  type = po_type;
  TreeItem = NULL;
  destroying = false;
  if (name_in == NULL) name_in = typetext();
  name = strdup(name_in);
  parent_obj = NULL;
  current_child = NULL;
  TreeAllocItem();
}

plot_obj::~plot_obj() {
  free(name);
  name = NULL;
  if (this == Current::Menu_obj)
	Current::Menu_obj = NULL;
  if (parent_obj == NULL || !parent_obj->destroying)
	TreeFreeItem();
}

const char *plot_obj::typetext() {
  switch ( type ) {
    case po_figure: return "figure";
    case po_pane: return "pane";
    case po_axes: return "axes";
    case po_data: return "graph";
    case po_line: return "line";
    case po_text: return "text";
    default: return "unknown";
  }
}

void plot_obj::got_focus(focus_source whence) {
  const char *twhence;
  switch (whence) {
  case focus_from_user: twhence = "user"; break;
  case focus_from_child: twhence = "child"; break;
  case focus_from_parent: twhence = "parent"; break;
  default: nl_error(4, "Invalid whence");
  }
  nl_error(0, "%s %s got focus from %s", typetext(), name, twhence);
  if (whence == focus_from_user) {
	nl_assert(TreeItem != NULL);
	if ( !(TreeItem->gen.list.flags&Pt_LIST_ITEM_SELECTED)) {
	  PtTreeSelect(ABW_Graphs_Tab, TreeItem);
	}
  }
  if (parent_obj && (whence == focus_from_user || whence == focus_from_child)) {
	parent_obj->current_child = this;
	parent_obj->got_focus(focus_from_child);
  }
  if (whence == focus_from_user || whence == focus_from_parent) {
	if (current_child) current_child->got_focus(focus_from_parent);
	else Current::none(type);
  }
}

void plot_obj::TreeAllocItem() {
  char temp_buf[80];
  if ( snprintf(temp_buf, 80, "%s\t%s", name, typetext()) >= 80 )
	nl_error(2,"Variable name exceeds buffer length in TreeAllocItem");
  TreeItem = PtTreeAllocItem(ABW_Graphs_Tab, temp_buf, -1, -1);
  TreeItem->data = (void *)this;
}

void plot_obj::TreeFreeItem() {
  if (TreeItem) {
    PtTreeRemoveItem(ABW_Graphs_Tab, TreeItem);
    PtTreeFreeItems(TreeItem);
    TreeItem = NULL;
  }
}

int plot_obj::pt_got_focus( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
  plot_obj *po;
  /* eliminate 'unreferenced' warnings */
  apinfo = apinfo, cbinfo = cbinfo;
  PtGetResource(widget, Pt_ARG_POINTER, &po, 0);
  if (po != NULL)
	  po->got_focus(focus_from_user);
  return Pt_CONTINUE;
}

int plot_obj::TreeSelected( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
  /* eliminate 'unreferenced' warnings */
  widget = widget, apinfo = apinfo;
  if (cbinfo->reason_subtype == Pt_LIST_SELECTION_FINAL) {
	PtTreeCallback_t *cb = (PtTreeCallback_t *)cbinfo->cbdata;
	nl_assert(cb->item != NULL);
	plot_obj *p = (plot_obj *)cb->item->data;
	if (cb->item->gen.list.flags&Pt_LIST_ITEM_SELECTED) {
	  p->got_focus(focus_from_user);
      nl_error( 0, "Selected %s:%s", p->name, p->typetext());
	}
  }
  return( Pt_CONTINUE );
}

int plot_obj::menu_ToggleVisible( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
  /* eliminate 'unreferenced' warnings */
  widget = widget, apinfo = apinfo, cbinfo = cbinfo;
  nl_assert(Current::Menu_obj != NULL);
  nl_error(0,"plot_obj: ToggleVisible %s:%s", Current::Menu_obj->name, Current::Menu_obj->typetext());
  Current::Menu_obj = NULL;
  return( Pt_CONTINUE );
}

int plot_obj::menu_Delete( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
  /* eliminate 'unreferenced' warnings */
  widget = widget, apinfo = apinfo, cbinfo = cbinfo;
  nl_assert(Current::Menu_obj != NULL);
  nl_error(0,"plot_obj: Delete %s:%s", Current::Menu_obj->name, Current::Menu_obj->typetext());
  delete Current::Menu_obj;
  Current::Menu_obj = NULL;
  return( Pt_CONTINUE );
}

int plot_obj::context_menu_setup( PtWidget_t *link_instance,
		ApInfo_t *apinfo, PtCallbackInfo_t *cbinfo ) {
  /* eliminate 'unreferenced' warnings */
  link_instance = link_instance, apinfo = apinfo, cbinfo = cbinfo;
  nl_assert(Current::Menu_obj != NULL);
  
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
		if (item != NULL) {
		  plot_obj *po = (plot_obj *)item->data;
		  Current::Menu_obj = po;
		  nl_error( 0, "plot_obj: Menu: %s:%s", po->name, po->typetext());
		  ApCreateModule (ABM_plot_context_menu, widget, cbinfo);
		}
	  }
	}
	return( Pt_CONTINUE );
}
