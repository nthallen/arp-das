/* Header "phrtg.h" for phrtg Application */
#ifndef PHRTG_H_INCLUDED
#define PHRTG_H_INCLUDED
#include "nortlib.h"
#include "tm.h"
#include <Pt.h>
#include <photon/PtTree.h>

class RTG_Variable_Node;
class RTG_Variable_MLF;
enum RTG_Variable_Type { Var_Node, Var_MLF };
const int DIV_BEVEL_WIDTHS = 2;

class RTG_Variable {
  public:
    RTG_Variable(const char *name_in, RTG_Variable_Type type_in);
    ~RTG_Variable();
    void Add_Sibling(RTG_Variable *newsib);
    RTG_Variable_Type type;
    static int Find_Insert( char *name, RTG_Variable_Node *&parent,
        RTG_Variable *&sib, RTG_Variable *&node, char *&lastnode_text );
    static int TreeSelected( PtWidget_t *widget,
    	ApInfo_t *apinfo, PtCallbackInfo_t *cbinfo );
    static RTG_Variable *Cur_Var;
    char *name;
  friend class RTG_Variable_Node;
  friend class RTG_Variable_MLF;
  protected:
    static RTG_Variable *Root;
    void update_ancestry( RTG_Variable_Node *parent_in, RTG_Variable *sib );
    RTG_Variable_Node *Parent;
    RTG_Variable *Next;
    PtTreeItem_t *TreeItem;
};

class RTG_Variable_Node : public RTG_Variable {
  public:
    RTG_Variable_Node(const char *name);
    //~RTG_Variable_Node();
    void Add_Child(RTG_Variable *child);
    friend class RTG_Variable;
  private:
    RTG_Variable *First;
};

class RTG_Variable_Data : public RTG_Variable {
  public:
    RTG_Variable_Data(const char *name_in, RTG_Variable_Type type_in);
};

class RTG_Variable_MLF : public RTG_Variable_Data {
  public:
    RTG_Variable_MLF( const char *name_in );
    void new_index(int index);
    static void set_default_path(const char *path_in);
    static void Incoming( char *name, int index );
  private:
    static char *default_path;
    char *path;
};

enum plot_obj_type { po_figure, po_pane, po_axes, po_data,
		po_line, po_text, po_zoom, po_max };

class plot_pane;
class plot_figure;
class plot_obj {
  public:
	plot_obj_type type;
	char *name;
    PtTreeItem_t *TreeItem;
	plot_obj( plot_obj_type po_type, const char *name_in);
	void TreeAllocItem();
	const char *typetext();
	static int TreeSelected( PtWidget_t *widget, ApInfo_t *apinfo,
			PtCallbackInfo_t *cbinfo );
	static int ToggleVisible( PtWidget_t *widget, ApInfo_t *apinfo,
			PtCallbackInfo_t *cbinfo );
	static int Delete( PtWidget_t *widget, ApInfo_t *apinfo,
			PtCallbackInfo_t *cbinfo );
	static int context_menu_setup( PtWidget_t *link_instance,
			ApInfo_t *apinfo, PtCallbackInfo_t *cbinfo );
	static int TreeColSelect( PtWidget_t *widget, ApInfo_t *apinfo,
			PtCallbackInfo_t *cbinfo );
	static int TreeInput( PtWidget_t *widget, ApInfo_t *apinfo,
			PtCallbackInfo_t *cbinfo );
};

class plot_figure : public plot_obj {
  public:
	plot_figure( const char *name_in );
	plot_pane *first;
	plot_pane *last;
	plot_figure *next;
	static int Setup( PtWidget_t *link_instance, ApInfo_t *apinfo,
			PtCallbackInfo_t *cbinfo );
	static int Realized( PtWidget_t *widget, ApInfo_t *apinfo,
			PtCallbackInfo_t *cbinfo );
	static int divider_resized( PtWidget_t *widget, ApInfo_t *apinfo,
			PtCallbackInfo_t *cbinfo );
	static int divider_drag( PtWidget_t *widget, ApInfo_t *apinfo,
			PtCallbackInfo_t *cbinfo );
	static void Report();
	int Resized(PtContainerCallback_t *cb);
	void Change_min_dim(int dw, int dh);
	bool display_name;
	bool visible;
	PtWidget_t *module;
	PtWidget_t *window;
  private:
	bool saw_first_resize;
	PhDim_t dim, min_dim;
	// Area: x,y,height,width (controlled by dragging, not dialog, but should be saved with configuration)
	// [Pane Orientation: Vertical]
	// [other window display functions: allow resize, minimize, maximize]
	// [[*Background_Color: Color Should be a Pane Property]]
	
};

class plot_pane : public plot_obj {
  public:
	plot_pane( const char *name_in, plot_figure *parent, PtWidget_t *pane = NULL);
	void resized( PhDim_t *newdim );
	//plot_axes *first;
	//plot_axes *last;
	plot_pane *next;
	plot_figure *parent;
	PtWidget_t *widget;
	int full_height;
	int min_height;
};

extern plot_figure *Cur_Figure, *All_Figures;
#endif
