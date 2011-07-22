/** \file phrtg.h
 * Main Header for phrtg Application
 */
/** \mainpage PhRTG Documentation
 * PhRTG provides realtime graphing capability to the
 * <a href="http://www.arp.harvard.edu/eng/das/index.html">Anderson Group Data Acquisition System</a>
 * as implemented under QNX6 and the Photon MicroGUI.
 */
#ifndef PHRTG_H_INCLUDED
#define PHRTG_H_INCLUDED
#include "nortlib.h"
#include "tm.h"
#include "f_matrix.h"
#include "mlf.h"
#include <Pt.h>
#include <photon/PtTree.h>
#include <list>
#include <vector>
#include <deque>
#include <fftw3.h>

#define MAX_VAR_LENGTH 80
#define MAX_VAR_NODE_LENGTH 40
#define MAX_VAR_NODES 6

class RTG_Limits;
class RTG_Variable_Node;
class RTG_Variable_MLF;
class RTG_Variable_Derived;
class plot_pane;
class plot_figure;
class plot_axes;
class plot_graph;
class plot_line;
class RTG_Cfg_Ser;
enum RTG_Variable_Type { Var_Node, Var_MLF, Var_Detrend,
      Var_Invert, Var_FFT, Var_FFT_PSD, Var_FFT_Phase,
      Var_Trend };
const int DIV_BEVEL_WIDTHS = 2;

class RTG_Range {
  public:
    double min, max;
    bool range_required;
    bool range_is_current;
    bool range_is_empty;
    bool range_updated;
    RTG_Range();
    void clear();
    //void check_required(RTG_Limits &lims);
    void update(double min_in, double max_in);
    inline void update(double val ) { update(val, val); }
    void update(RTG_Range &R);
    bool changed(RTG_Range &R);
};

class RTG_Limits {
  public:
    double min, max;
    double units_per_Mtick;
    double span;
    /**
     * Unlike in RTG_Range, epoch in RTG_Limits is not arbitrary.
     * In a trend plot, it will be the true time value corresponding
     * to the right hand limit. This value is subtracted from all
     * X values before plotting, which means max is always 0.
     * and min is always -span.
     */
    double epoch;
    // bool range_required;
    /**
     * Overrides limits_auto. Indicates data should be plotted
     * relative to epoch. On update(), epoch should be
     * redefined to be equal to the maximum data range.
     * Hence max will be zero.
     */
    bool limits_trend;
    bool limits_auto;
    bool limits_current;
    bool limits_empty;
    RTG_Limits();
    // void clear();
    // void update(scalar_t min_in, scalar_t max_in);
    // inline void update(scalar_t val ) { update(val, val); }
    // void update(RTG_Range &R);
    bool changed(RTG_Range &R);
};

class RTG_Variable {
  public:
    RTG_Variable_Type type;
    char *name;
    bool destroying;

    RTG_Variable(const char *name_in, RTG_Variable_Type type_in);
    virtual ~RTG_Variable();
    void AddSibling(RTG_Variable *newsib);
    bool snprint_path(char *dest, int n);
    virtual bool reload() = 0;

    static int Find_Insert( const char *name, RTG_Variable_Node *&parent,
        RTG_Variable *&sib, RTG_Variable *&node, const char *&lastnode_text );
    static int TreeSelected( PtWidget_t *widget,
      ApInfo_t *apinfo, PtCallbackInfo_t *cbinfo );
    static bool reload_all();

  friend class RTG_Variable_Node;
  friend class RTG_Variable_MLF;
  friend class RTG_Variable_Detrend;
  friend class RTG_Variable_Invert;
  friend class RTG_Variable_FFT;
  friend class RTG_Variable_PSD;
  friend class RTG_Variable_Phase;
  protected:
    static RTG_Variable *Root;
    RTG_Variable_Node *Parent;
    RTG_Variable *Next;
    PtTreeItem_t *TreeItem;
    void update_ancestry( RTG_Variable_Node *parent_in, RTG_Variable *sib );
    bool isnprint_path(char *&dest, int &n);
};

/**
 * RTG_Variable_Node represents an internal node in the tree structure
 * displayed on the Variable tab. It does not have any data associated
 * with it, but it's descencdents do.
 */
class RTG_Variable_Node : public RTG_Variable {
  public:
    RTG_Variable_Node(const char *name, RTG_Variable_Node *parent_in, RTG_Variable *sib );
    ~RTG_Variable_Node();
    void Add_Child(RTG_Variable *child);
    void Remove_Child(RTG_Variable *child);
    bool reload();
    friend class RTG_Variable;
  private:
    RTG_Variable *First;
};

/**
 * RTG_Variable_Data is the virtual base class for leaf nodes in the
 * tree displayed on the Variable tab. The only current implementation
 * is RTG_Variable_Matrix, but I expect RTG_Variable_Trend to be
 * another inheritor.
 */
class RTG_Variable_Data : public RTG_Variable {
  public:
    bool new_data_available;
    /** \brief We know there is new data to be processed
     * When true, reload_data() will be called once, and if it
     * returns true, graphs will have new_data set to true,
     * and derivatives will have reload_required set to true.
     */
    bool reload_required;
    /** \brief true if y_vector() method is useful
     */
    bool has_y_vector;
    unsigned nrows, ncols;
    std::list<plot_graph*> graphs;
    std::list<RTG_Variable_Derived *> derivatives;

    RTG_Variable_Data(const char *name_in, RTG_Variable_Type type_in);
    ~RTG_Variable_Data();
    virtual void AddGraph(plot_graph *graph);
    virtual void RemoveGraph(plot_graph *graph);
    void AddDerived(RTG_Variable_Derived *var);
    virtual void RemoveDerived(RTG_Variable_Derived *var);

    /**
     * @return True if something useful is accomplished.
     */
    virtual bool check_for_updates();

    /**
     * @return True if something useful is accomplished.
     */
    bool reload();

    /**
     * Called when the data needs to be updated before
     * being used. If the data in columns can be updated
     * independently, we can just set flags to be checked
     * when the data is accessed.
     * @return True if data is successfully updated.
     */
    virtual bool reload_data() = 0;
    virtual bool get(unsigned r, unsigned c, double &X, double &Y) = 0;
    
    /**
     * If X.range_required is false and X.range_is_current is true,
     * the (fixed) X range can be used to constrain the Y values
     * for the range calculation.
     */
    virtual void evaluate_range(unsigned col, RTG_Range &X,
        RTG_Range &Y) = 0;

    /**
     * On exit, i_min and i_max are set to the sample
     * numbers that best represent the specified X range.
     * If the range is empty, i_min > i_max.
     * @param x_min The minimum value of the X range
     * @param x_max The maximum value of the X range
     * @param i_min The minimum index for the X range
     * @param i_max The maximum index for the X range
     */
    virtual void xrow_range(scalar_t x_min, scalar_t x_max,
        unsigned &i_min, unsigned &i_max) = 0;
    virtual vector_t y_vector(unsigned col) = 0;
    virtual RTG_Variable_Data *Derived_From();
};

class RTG_Variable_Matrix : public RTG_Variable_Data {
  public:
    f_matrix data;
    RTG_Variable_Matrix(const char *name_in, RTG_Variable_Type type_in);
    bool get(unsigned r, unsigned c, double &X, double &Y);
    void evaluate_range(unsigned col, RTG_Range &X,
         RTG_Range &Y);
    vector_t y_vector(unsigned col);
};

class RTG_Variable_MLF : public RTG_Variable_Matrix {
  public:
    RTG_Variable_MLF( const char *name_in, RTG_Variable_Node *parent_in, RTG_Variable *sib );
    bool reload_data();

    /**
     * On exit, i_min and i_max are set to the sample
     * numbers that best represent the specified X range.
     * If the range is empty, i_min > i_max.
     * @param x_min The minimum value of the X range
     * @param x_max The maximum value of the X range
     * @param i_min The minimum index for the X range
     * @param i_max The maximum index for the X range
     */
    void xrow_range(scalar_t x_min, scalar_t x_max,
            unsigned &i_min, unsigned &i_max);

    static void set_default_path(const char *path_in);
    static void Incoming( const char *name, unsigned long index );
  private:
    static char *default_path;
    mlf_def_t *mlf;
    unsigned long next_index;
    void new_index(unsigned long index);
};

class trend_queue : public std::deque<scalar_t> {
  public:
    /** The number of Y columns */
    unsigned n_cols;
    double x_max, x_min, x_epoch, span;
    static const int MINPOINTS = 500;
    
    trend_queue();
    /** Guarantees monotonicity, column coherency */
    void push( unsigned nc, double X, double *Y );
    /** Copy all data into destination */
    void flush( trend_queue &dest );
    /** Retire old data based on span and minimum allocation */
    void flush();
    int n_rows();
    /**
     * Sets X and Y to the values for the specified row and column.
     * The X value is adjusted to be relative to the specified epoch.
     * @return true on success, false if indices are out of range
     */
    bool get(unsigned r, unsigned c, double &X, double &Y, double epoch);
};

class RTG_Variable_Trend : public RTG_Variable_Data {
  public:
    RTG_Variable_Trend(const char *name_in, RTG_Variable_Node *parent_in, RTG_Variable *sib);
    bool reload_data();
    bool get(unsigned r, unsigned c, double &X, double &Y);
    void evaluate_range(unsigned col, RTG_Range &X,
        RTG_Range &Y);
    /**
     * On exit, i_min and i_max are set to the sample
     * numbers that best represent the specified X range.
     * If the range is empty, i_min > i_max.
     * @param x_min The minimum value of the X range
     * @param x_max The maximum value of the X range
     * @param i_min The minimum index for the X range
     * @param i_max The maximum index for the X range
     */
    void xrow_range(scalar_t x_min, scalar_t x_max,
        unsigned &i_min, unsigned &i_max);
    vector_t y_vector(unsigned col);
    static void Incoming( const char *cmd );
  private:
    trend_queue pending, data;
};

class RTG_Variable_Derived : public RTG_Variable_Matrix {
  public:
    RTG_Variable_Data *Source;
    
    RTG_Variable_Derived(RTG_Variable_Data *src, const char *name_in,
        RTG_Variable_Type type_in);
    ~RTG_Variable_Derived();
    bool reload_data();
    bool get(unsigned r, unsigned c, double &X, double &Y);
    vector_t y_vector(unsigned col);
    void RemoveGraph(plot_graph *graph);
    void RemoveDerived(RTG_Variable_Derived *var);
    bool check_for_updates();
    RTG_Variable_Data *Derived_From();
    virtual void derive(unsigned c) = 0;
  protected:
    std::vector<bool> derive_required;
};

class RTG_Variable_Detrend : public RTG_Variable_Derived {
  public:
    RTG_Variable_Detrend(RTG_Variable_Data *src, const char *name_in,
        RTG_Variable_Node *parent_in, RTG_Variable *sib,
        scalar_t min, scalar_t max);
    void xrow_range(scalar_t x_min, scalar_t x_max,
            unsigned &i_min, unsigned &i_max);
    bool reload_data();
    void derive(unsigned c);
    static RTG_Variable_Detrend *Create( RTG_Variable_Data *src,
            scalar_t min, scalar_t max );
  private:
    scalar_t x_min, x_max;
    unsigned i_min, i_max;
};

class RTG_Variable_Invert : public RTG_Variable_Derived {
  public:
    RTG_Variable_Invert(RTG_Variable_Data *src, const char *name_in,
	RTG_Variable_Node *parent_in, RTG_Variable *sib );
    bool reload_data();
    void xrow_range(scalar_t x_min, scalar_t x_max,
            unsigned &i_min, unsigned &i_max);
    void derive(unsigned col);
    static RTG_Variable_Invert *Create( RTG_Variable_Data *src );
};

class FFT_Plan {
  public:
    FFT_Plan();
    ~FFT_Plan();
    void fft(scalar_t *iv, scalar_t *ov, int Npts);
  private:
    fftwf_plan P;
    scalar_t *ivec;
    scalar_t *ovec;
    int N;
};

class RTG_Variable_FFT : public RTG_Variable_Derived {
  public:
    RTG_Variable_FFT(RTG_Variable_Data *src, const char *name_in,
	RTG_Variable_Node *parent_in, RTG_Variable *sib,
        scalar_t min, scalar_t max);
    void AddGraph(plot_graph *graph);
    bool reload_data();
    void derive(unsigned col);
    void xrow_range(scalar_t x_min, scalar_t x_max,
            unsigned &i_min, unsigned &i_max);
    static RTG_Variable_FFT *Create( RTG_Variable_Data *src,
            scalar_t min, scalar_t max );
  private:
    scalar_t x_min, x_max;
    unsigned i_min, i_max;
    int Ni;
    std::vector<FFT_Plan> plans;
};

class RTG_Variable_PSD : public RTG_Variable_Derived {
  public:
    RTG_Variable_PSD(RTG_Variable_Data *src, const char *name_in,
	RTG_Variable_Node *parent_in, RTG_Variable *sib);
    bool reload_data();
    void derive(unsigned col);
    void xrow_range(scalar_t x_min, scalar_t x_max,
            unsigned &i_min, unsigned &i_max);
    static RTG_Variable_PSD *Create( RTG_Variable_Data *src,
            scalar_t min, scalar_t max );
  private:
};

class RTG_Variable_Phase : public RTG_Variable_Derived {
  public:
    RTG_Variable_Phase(RTG_Variable_Data *src, const char *name_in,
	RTG_Variable_Node *parent_in, RTG_Variable *sib);
    bool reload_data();
    void derive(unsigned col);
    void xrow_range(scalar_t x_min, scalar_t x_max,
            unsigned &i_min, unsigned &i_max);
    static RTG_Variable_Phase *Create( RTG_Variable_Data *src,
            scalar_t min, scalar_t max );
  private:
};

enum plot_obj_type { po_root, po_figure, po_pane, po_axes, po_data,
		po_line, po_text, po_zoom, po_max };
enum focus_source { focus_from_user, focus_from_child, focus_from_parent };
enum Update_Source { from_file, from_widget };


class plot_obj {
  public:
  	plot_obj_type type;
  	char *name;
    PtTreeItem_t *TreeItem;
    plot_obj *parent_obj;
    plot_obj *current_child;
    bool visible;
    bool new_visibility;
    bool destroying;

    plot_obj( plot_obj_type po_type, const char *name_in);
  	virtual ~plot_obj();
  	virtual void got_focus(focus_source whence);
  	void TreeAllocItem();
  	void TreeFreeItem();
  	virtual void rename(const char *text, Update_Source src);
  	const char *typetext();
  	virtual plot_obj *default_child();

	static bool rendering;
	static bool background_set;
	static PtWorkProcId_t *WorkProcId;
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
	static void setup_background();
	static void render_all();
	static bool render_each();
	static void render_one();
  static bool check_vars_for_updates();
};

class plot_figure : public plot_obj {
  public:
  	std::list<plot_pane*> panes;
  	bool resizing;
  	bool display_name;
  	bool synch_x;
  	PtWidget_t *module;
  	PtWidget_t *window;
    PhDim_t dim, min_dim;
  
  	plot_figure( const char *name_in );
  	~plot_figure();
  	void AddChild(plot_pane *p);
  	void RemoveChild(plot_pane *p);
  	plot_pane *CreateGraph(RTG_Variable_Data *var);
  	int resized(PhDim_t *old_dim, PhDim_t *new_dim, bool force);
  	void Adjust_Panes(int delta_min_height);
  	void Change_min_dim(int dw, int dh);
  	void got_focus(focus_source whence);
  	void rename(const char *text, Update_Source src);
  	void Update_Window_Tab();
    plot_obj *default_child();
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
  	PhPoint_t Pos;
  	// Area: x,y,height,width (controlled by dragging, not dialog, but should be saved with configuration)
  	// [Pane Orientation: Vertical]
  	// [other window display functions: allow resize, minimize, maximize]
  	// [[*Background_Color: Color Should be a Pane Property]]
};

extern std::list<plot_figure*> All_Figures;

enum Axis_XY { Axis_X, Axis_Y };
enum Tab_Name { Tab_None, Tab_Variables, Tab_Graphs, Tab_Figure,
  Tab_X, Tab_Y, Tab_Line };

class plot_pane : public plot_obj {
  public:
    std::list<plot_axes*> axes;
		plot_figure *parent;
		PtWidget_t *widget;
		PgColor_t pane_color;
		int full_height;
		int min_height;
		int full_width;
		bool synch_x;
		
  	plot_pane( const char *name_in, plot_figure *parent);
  	~plot_pane();
  	void AddChild(plot_axes *p);
  	void RemoveChild(plot_axes *p);
  	plot_axes *CreateGraph(RTG_Variable_Data *var);
  	void resized( PhDim_t *newdim );
  	void got_focus(focus_source whence);
    void rename(const char *text, Update_Source src);
    void set_bg_color(PgColor_t rgb, Update_Source src);
  	bool render();
    plot_obj *default_child();
    bool check_for_updates(bool parent_visibility);
    void Update_Axis_Pane();
    
    static PtWidget_t *cache;
};

class plot_axis {
  public:
  	Axis_XY XY;
  	bool reverse_dim; // based on X or Y
    bool reverse; // user-selectable
  	bool data_range_updated;
  	// bool axis_range_updated;
  	bool axis_limits_updated;
    bool axis_limits_trended;
  	bool draw[2]; // Whether to draw primary or secondary axis
  	bool reserve_tick_space[2];
  	bool draw_ticks[2];
  	bool reserve_tick_label_space[2];
  	bool draw_tick_label[2];
  	bool reserve_label_space[2];
  	bool draw_label[2];
  	bool log_scale;
  	RTG_Range range;
  	RTG_Limits limits;
  	int pixels;
  	float scalev;
  	float clip_max;
  	float clip_min;
      // *Axis color: color
      // *Axis Tick Label Style: *use defaults for now
      // *Axis Label Style: *use defaults for now
  	int major_tick_len; // positive outward, negative inward
  	int minor_tick_len;
  	int label_height; // same units has *_tick_len.
  	static const int pane_overage;

  	plot_axis();
  	void check_limits();
    void set_scale();
    void set_scale(int pixel_span);
    void set_scale(double min, double max);
    short evaluate(scalar_t V);
    bool render(plot_axes *axes);
    void Update_Axis_Pane(plot_axes *parent);
    void Update_Axis_Pane_Limits();
    
    static void Clear_Axis_Pane();
};

class plot_axes : public plot_obj {
  public:
    plot_pane *parent;
    plot_axis X;
    plot_axis Y;
    std::list<plot_graph*> graphs;
    bool detrended;
    bool inverted;
    bool psd_transformed;
    bool ph_transformed;
    
    plot_axes( const char *name_in, plot_pane *parent );
    ~plot_axes();
    void AddChild(plot_graph *p);
    void RemoveChild(plot_graph *p);
    plot_graph *CreateGraph(RTG_Variable_Data *var);
    void got_focus(focus_source whence);
    void resized( PhDim_t *newdim );
    bool check_limits();
    void Update_Axis_Pane();
    void rename(const char *text, Update_Source src);
  	virtual bool render();
    plot_obj *default_child();
    bool check_for_updates(bool parent_visibility);
    void schedule_range_check();
    void Detrend(long value);
    void Invert(long value);
    void PSD(long value);
    void Phase(long value);
};

class plot_axes_diag : public plot_axes {
  public:
    std::vector<PhPoint_t> idata;
    PtWidget_t *widget;
    int Xpx, Ypx;
    static const int Nlvls;

    plot_axes_diag( const char *name_in, plot_pane *parent );
    ~plot_axes_diag();
    bool render();
  private:
    void draw(int side, int x0, int y0, int dx, int dy);
};

class plot_graph : public plot_obj {
  public:
    bool new_data;
    bool axes_rescaled;
    bool x_axis_trended;
    bool redraw_required;
    bool check_range;
    plot_axes *parent;
    RTG_Variable_Data *variable;
    std::vector<plot_line*> lines;

    plot_graph(RTG_Variable_Data *var, plot_axes *parent);
  	~plot_graph();
  	void got_focus(focus_source whence);
  	bool check_limits( RTG_Range &Xr, RTG_Range &Yr );
    bool render();
    plot_obj *default_child();
    bool check_for_updates(bool parent_visibility);
    void rename(const char *text, Update_Source src);
};

class plot_line : public plot_obj {
  public:
    bool new_data;
    bool redraw_required;
    bool x_axis_trended;
    bool check_range;
    bool effective_visibility;
    plot_graph *parent;
    int column;
    PgColor_t color;
    RTG_Range Xrange, Yrange;
    std::vector<PhPoint_t> idata;
    std::vector<PtWidget_t *> widgets;

    plot_line(plot_graph *parent_in, unsigned col, const char *name_in);
  	~plot_line();
  	void clear_widgets();
  	void got_focus(focus_source whence);
  	bool check_limits( RTG_Range &Xr, RTG_Range &Yr );
    void set_line_color(PgColor_t rgb, Update_Source src);
    bool render();
    bool check_for_updates(bool parent_visibility);
    void Update_Line_Tab();
  	static plot_line *new_line(plot_graph *parent_in, unsigned col);
  	static const unsigned pts_per_polygon;
};

class Current {
  public:
  	static RTG_Variable_Data *Variable;
    static plot_figure *Figure;
    static plot_pane *Pane;
    static plot_axes *Axes;
    static plot_axis *Axis;
    static plot_graph *Graph;
    static plot_line *Line;
    static plot_obj *Menu_obj;
    static Tab_Name Tab;
    static void none(plot_obj_type parent_type);
};

class RTG_Cfg {
  public:
    static int Serialize( PtWidget_t *widget, ApInfo_t *apinfo,
      PtCallbackInfo_t *cbinfo );
    static void Load( PtWidget_t *widget, ApInfo_t *apinfo,
      PtCallbackInfo_t *cbinfo);
};

extern void Update_Text( int Name, char *text, Update_Source src );
void Update_Color(int Name, PgColor_t rgb, Update_Source src );
void Update_Toggle(int Name, long int value, Update_Source src );
void Update_Numeric(int Name, double value, Update_Source src );
#endif
