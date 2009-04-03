/* Header "phrtg.h" for phrtg Application */
#ifndef PHRTG_H_INCLUDED
#define PHRTG_H_INCLUDED
#include "nortlib.h"
#include "tm.h"
#include "f_matrix.h"
#include "mlf.h"
#include <Pt.h>
#include <photon/PtTree.h>
#include <list>

class RTG_Variable_Node;
class RTG_Variable_MLF;
class plot_pane;
class plot_figure;
class plot_axes;
class plot_data;
class plot_line;
enum RTG_Variable_Type { Var_Node, Var_MLF };
const int DIV_BEVEL_WIDTHS = 2;

class RTG_Variable {
  public:

    RTG_Variable_Type type;
    char *name;

    RTG_Variable(const char *name_in, RTG_Variable_Type type_in);
    ~RTG_Variable();
    void AddSibling(RTG_Variable *newsib);
    void AddGraph(plot_data *graph);
    void RemoveGraph(plot_data *graph);
    virtual bool reload() = 0;

    static int Find_Insert( char *name, RTG_Variable_Node *&parent,
        RTG_Variable *&sib, RTG_Variable *&node, char *&lastnode_text );
    static int TreeSelected( PtWidget_t *widget,
      ApInfo_t *apinfo, PtCallbackInfo_t *cbinfo );
    static bool reload_all();

  friend class RTG_Variable_Node;
  friend class RTG_Variable_MLF;
  protected:
    static RTG_Variable *Root;
    RTG_Variable_Node *Parent;
    RTG_Variable *Next;
    PtTreeItem_t *TreeItem;
    std::list<plot_data*> graphs;
    void update_ancestry( RTG_Variable_Node *parent_in, RTG_Variable *sib );
};

class RTG_Variable_Node : public RTG_Variable {
  public:
    RTG_Variable_Node(const char *name);
    //~RTG_Variable_Node();
    void Add_Child(RTG_Variable *child);
    bool reload();
    friend class RTG_Variable;
  private:
    RTG_Variable *First;
};

class RTG_Variable_Data : public RTG_Variable {
  public:
    bool new_data_available;
    bool reload_required;
    RTG_Variable_Data(const char *name_in, RTG_Variable_Type type_in);
    bool check_for_updates();
    virtual bool reload() = 0;
};

class RTG_Variable_MLF : public RTG_Variable_Data {
  public:
    RTG_Variable_MLF( const char *name_in );
    bool reload();

    static void set_default_path(const char *path_in);
    static void Incoming( char *name, unsigned long index );
  private:
    static char *default_path;
    mlf_def_t *mlf;
    unsigned long next_index;
    void new_index(unsigned long index);
};

enum plot_obj_type { po_figure, po_pane, po_axes, po_data,
		po_line, po_text, po_zoom, po_max };
enum focus_source { focus_from_user, focus_from_child, focus_from_parent };


class plot_obj {
  public:
	plot_obj_type type;
	char *name;
    PtTreeItem_t *TreeItem;
    plot_obj *parent_obj;
    plot_obj *current_child;
    bool destroying;
	plot_obj( plot_obj_type po_type, const char *name_in);
	virtual ~plot_obj();
	virtual void got_focus(focus_source whence);
	void TreeAllocItem();
	void TreeFreeItem();
	const char *typetext();
	static int TreeSelected( PtWidget_t *widget, ApInfo_t *apinfo,
			PtCallbackInfo_t *cbinfo );
	static int menu_ToggleVisible( PtWidget_t *widget, ApInfo_t *apinfo,
			PtCallbackInfo_t *cbinfo );
	static int menu_Delete( PtWidget_t *widget, ApInfo_t *apinfo,
			PtCallbackInfo_t *cbinfo );
	static int context_menu_setup( PtWidget_t *link_instance,
			ApInfo_t *apinfo, PtCallbackInfo_t *cbinfo );
	static int TreeColSelect( PtWidget_t *widget, ApInfo_t *apinfo,
			PtCallbackInfo_t *cbinfo );
	static int TreeInput( PtWidget_t *widget, ApInfo_t *apinfo,
			PtCallbackInfo_t *cbinfo );
	static int pt_got_focus( PtWidget_t *widget, ApInfo_t *apinfo,
			PtCallbackInfo_t *cbinfo );
	static bool render_all();
  static bool check_vars_for_updates();
};

class plot_figure : public plot_obj {
  public:
  	std::list<plot_pane*> panes;
  	bool resizing;
  	bool display_name;
  	bool visible;
  	bool synch_x;
  	PtWidget_t *module;
  	PtWidget_t *window;
  
  	plot_figure( const char *name_in );
  	~plot_figure();
  	void AddChild(plot_pane *p);
  	void RemoveChild(plot_pane *p);
  	void CreateGraph(RTG_Variable_Data *var);
  	int resized(PhDim_t *old_dim, PhDim_t *new_dim, bool force);
  	void Change_min_dim(int dw, int dh);
  	void got_focus(focus_source whence);
  	bool render();
  	bool check_for_updates();
  
  	static int Setup( PtWidget_t *link_instance, ApInfo_t *apinfo,
  			PtCallbackInfo_t *cbinfo );
  	static int Realized( PtWidget_t *widget, ApInfo_t *apinfo,
  			PtCallbackInfo_t *cbinfo );
  	static int divider_resized( PtWidget_t *widget, ApInfo_t *apinfo,
  			PtCallbackInfo_t *cbinfo );
  	static int divider_drag( PtWidget_t *widget, ApInfo_t *apinfo,
  			PtCallbackInfo_t *cbinfo );
  	static int unrealized( PtWidget_t *widget, ApInfo_t *apinfo,
  			PtCallbackInfo_t *cbinfo );
  	static int destroyed( PtWidget_t *widget, ApInfo_t *apinfo,
  			PtCallbackInfo_t *cbinfo );
  	static int wmevent( PtWidget_t *widget, ApInfo_t *apinfo,
  			PtCallbackInfo_t *cbinfo );
  	static void Report();
  private:
  	bool saw_first_resize;
  	PhDim_t dim, min_dim;
  	// Area: x,y,height,width (controlled by dragging, not dialog, but should be saved with configuration)
  	// [Pane Orientation: Vertical]
  	// [other window display functions: allow resize, minimize, maximize]
  	// [[*Background_Color: Color Should be a Pane Property]]
};

extern std::list<plot_figure*> All_Figures;

class plot_pane : public plot_obj {
  public:
    std::list<plot_axes*> axes;
		plot_figure *parent;
		PtWidget_t *widget;
		int full_height;
		int min_height;
		bool synch_x;
		
  	plot_pane( const char *name_in, plot_figure *parent, PtWidget_t *pane = NULL);
  	~plot_pane();
  	void AddChild(plot_axes *p);
  	void RemoveChild(plot_axes *p);
  	void CreateGraph(RTG_Variable_Data *var);
  	void resized( PhDim_t *newdim );
  	void got_focus(focus_source whence);
  	bool render();
    bool check_for_updates();
};

enum Axis_XY { Axis_X, Axis_Y };

class plot_axis {
  public:
  	Axis_XY XY;
  	bool draw[2]; // Whether to draw primary or secondary axis
  	bool reserve_tick_space[2];
  	bool draw_ticks[2];
  	bool reserve_tick_label_space[2];
  	bool draw_tick_label[2];
  	bool reserve_label_space[2];
  	bool draw_label[2];
  	bool limit_auto;
  	bool log_scale;
  	bool reverse;
  	float min, max;
  	int pixels;
  	float scalev;
      // *Axis color: color
      // *Axis Tick Label Style: *use defaults for now
      // *Axis Label Style: *use defaults for now
  	int major_tick_len; // positive outward, negative inward
  	int minor_tick_len;
  	int label_height; // same units has *_tick_len.

  	plot_axis();
    void set_scale();
    void set_scale(int pixel_span);
    void set_scale(float min, float max);
    void set_scale(f_matrix *data);
};

class plot_axes : public plot_obj {
  public:
    bool visible;
    plot_pane *parent;
    plot_axis X;
    plot_axis Y;
    std::list<plot_data*> graphs;
    
  	plot_axes( const char *name_in, plot_pane *parent );
  	~plot_axes();
  	void AddChild(plot_data *p);
  	void RemoveChild(plot_data *p);
  	void CreateGraph(RTG_Variable_Data *var);
  	void got_focus(focus_source whence);
  	bool render();
    bool check_for_updates();
};

class plot_data : public plot_obj {
  public:
    bool visible;
    bool redraw_required;
    plot_axes *parent;
    RTG_Variable_Data *variable;
    // std::list<plot_line*> lines;

    plot_data(RTG_Variable_Data *var, plot_axes *parent);
  	~plot_data();
  	void got_focus(focus_source whence);
    bool render();
    bool check_for_updates();
};

class plot_line : public plot_obj {
  public:
    bool visible;
    plot_data *parent;
    int column;

    plot_line();
  	~plot_line();
  	void got_focus(focus_source whence);
};

class Current {
  public:
  	static RTG_Variable_Data *Variable;
    static plot_figure *Figure;
    static plot_pane *Pane;
    static plot_axes *Axes;
    static plot_data *Graph;
    static plot_line *Line;
    static plot_obj *Menu_obj;
    static void none(plot_obj_type parent_type);
};
#endif
