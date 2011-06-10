/** \file Variable.cc
 * Support routines for the Variable tab
 *
 * I should probably impose some arbitrary limits on name length.
 * Overall length should be less than 80.
 * Individual node length should not exceed 40.
 * Number of nodes in a name should not exceed 6.
 * Return non-zero on error.
 */
#include <ctype.h>
#include <math.h>
#include "ablibs.h"
#include "phrtg.h"
#include "abimport.h"
#include "nl_assert.h"

/**
 * The root for the Variable directory.
 */
RTG_Variable *RTG_Variable::Root;

RTG_Variable_Range::RTG_Variable_Range() {
  min = max = -1.;
  range_required = true;
  range_is_current = false;
  range_is_empty = true;
  range_auto = true;
}

void RTG_Variable_Range::update(scalar_t min_in, scalar_t max_in ) {
  if ( range_is_empty ) {
    min = min_in;
    max = max_in;
    range_is_empty = false;
  } else {
    if (min_in < min)
      min = min_in;
    if (max_in > max)
      max = max_in;
  }
}

void RTG_Variable_Range::update(RTG_Variable_Range &R ) {
  if (!R.range_is_empty)
    update(R.min, R.max);
}

/**
 * RTG_Variable_Range::changed sets the current range to that of the input
 * and returns true if this represents a change
 */
bool RTG_Variable_Range::changed(RTG_Variable_Range &R ) {
  range_is_current = true;
  range_required = false;
  if ( range_is_empty ) {
    if (R.range_is_empty) return false;
    min = R.min;
    max = R.max;
    range_is_empty = false;
    return true;
  }
  if (R.range_is_empty) {
    range_is_empty = true;
    return true;
  }
  if ( min == R.min && max == R.max )
    return false;
  min = R.min;
  max = R.max;
  return true;
}

void RTG_Variable_Range::clear() {
  range_is_empty = true;
}

RTG_Variable::RTG_Variable(const char *name_in, RTG_Variable_Type type_in) {
  nl_assert(name_in != NULL);
  Parent = NULL;
  Next = NULL;
  name = strdup(name_in);
  type = type_in;
  destroying = false;
  TreeItem = PtTreeAllocItem(ABW_Variables_Tab, name, -1, -1);
  TreeItem->data = (void *)this;
}

RTG_Variable::~RTG_Variable() {
  destroying = true;
  if (TreeItem) {
    PtTreeRemoveItem(ABW_Variables_Tab, TreeItem);
    PtTreeFreeItems(TreeItem);
    TreeItem = NULL;
  }
  free(name);
  name = NULL;
  if (Parent) Parent->Remove_Child(this);
  else if (Root == this) Root = Next;
  else {
    RTG_Variable *var = Root;
    for (var = Root; var != NULL; var = var->Next) {
      if (var->Next == this) {
        var->Next = Next;
        break;
      }
    }
  }
}

void RTG_Variable::AddSibling(RTG_Variable *newsib) {
  nl_assert(newsib != NULL);
  newsib->Next = this->Next;
  this->Next = newsib;
  newsib->Parent = this->Parent;
  PtTreeAddAfter(ABW_Variables_Tab, newsib->TreeItem, this->TreeItem);
}

void RTG_Variable::update_ancestry( RTG_Variable_Node *parent_in, RTG_Variable *sib ) {
  if ( sib ) {
    sib->AddSibling(this);
  } else if ( parent_in ) {
    parent_in->Add_Child(this);
  } else {
    this->Next = RTG_Variable::Root;
    RTG_Variable::Root = this;
    PtTreeAddFirst(ABW_Variables_Tab, this->TreeItem, NULL);
  }
}

bool RTG_Variable::reload_all() {
  if ( Root ) return Root->reload();
  return false;
}

/**
 * This is the Pt_CB_TREE_SELECTION callback for the Variables Tab
 */
int RTG_Variable::TreeSelected( PtWidget_t *widget, ApInfo_t *apinfo,
		PtCallbackInfo_t *cbinfo ) {
  if ( cbinfo->reason_subtype == Pt_LIST_SELECTION_FINAL ) {
	PtTreeCallback_t *PtTreeCallback = (PtTreeCallback_t *)cbinfo->cbdata;
	PtTreeItem_t *item = PtTreeCallback->item;
	RTG_Variable *var = (RTG_Variable *)item->data;
	if ( var == NULL || var->type == Var_Node ) {
	  Current::Variable = NULL;
	  PtTreeUnselect(ABW_Variables_Tab, item);
	} else {
	  Current::Variable = (RTG_Variable_Data *)var;
	  nl_error( -2, "Variable %s selected", var->name );
	}
  }
  return Pt_CONTINUE;
}

RTG_Variable_Node::RTG_Variable_Node(const char *name_in,
    RTG_Variable_Node *parent_in, RTG_Variable *sib )
    : RTG_Variable(name_in, Var_Node) {
  First = NULL;
  update_ancestry(parent_in, sib);
}

RTG_Variable_Node::~RTG_Variable_Node() {
  nl_assert(First == NULL);
}

void RTG_Variable_Node::Add_Child(RTG_Variable *child) {
  nl_assert(child != NULL);
  child->Next = this->First;
  this->First = child;
  child->Parent = this;
  PtTreeAddFirst( ABW_Variables_Tab, child->TreeItem, this->TreeItem );
}

void RTG_Variable_Node::Remove_Child(RTG_Variable *child) {
  nl_assert(child != NULL);
  if (First == child) {
    First = child->Next;
    if (First == NULL) delete this;
  } else {
    RTG_Variable *sib = First;
    while (sib != NULL) {
      if (sib->Next == child) {
        sib->Next = child->Next;
        break;
      }
    }
  }
}

bool RTG_Variable_Node::reload() {
  if (First && First->reload()) return true;
  if (Next) return Next->reload();
  return false;
}

#define MAX_VAR_LENGTH 80
#define MAX_VAR_NODE_LENGTH 40
#define MAX_VAR_NODES 6

int RTG_Variable::Find_Insert( char *name, RTG_Variable_Node *&parent,
    RTG_Variable *&sib, RTG_Variable *&node, char *&last_node_text ) {
  // Check name for valid syntax
  int node_start[MAX_VAR_NODES];
  int node_len[MAX_VAR_NODES];
  int n_nodes = 0;
  int i = 0;
  
  nl_assert( name != NULL );
  for (;;) {
    if ( n_nodes > MAX_VAR_NODES ) {
      nl_error( 2, "Variable name has too many nodes" );
      return 1;
    }
    node_start[n_nodes] = i;
    if (!isalpha(name[i])) {
      nl_error( 2, "Variable node must begin with alpha" );
      return 1;
    }
    while ( isalnum(name[i]) || name[i] == '_' ) ++i;
    if (name[i] == '/' || name[i] == '\0') {
      node_len[n_nodes] = i - node_start[n_nodes];
      if (node_len[n_nodes] > MAX_VAR_NODE_LENGTH) {
        nl_error( 2, "Variable node name exceeds limit" );
        return 1;
      }
      ++n_nodes;
      if (name[i] == '\0') break;
      ++i;
    } else {
      nl_error( 2, "Invalid character in variable name" );
    }
  }
  if ( i > MAX_VAR_LENGTH ) {
    nl_error( 2, "Variable name too long" );
    return 1;
  }
  // Now the syntax checks out, so we can start the search.
  
  parent = NULL;
  sib = NULL;
  node = NULL;
  
  for (i = 0; i < n_nodes; ++i ) {
    name[node_start[i]+node_len[i]] = '\0';
    char *node_name = &name[node_start[i]];
    RTG_Variable *var;
    for (var = parent ? parent->First : RTG_Variable::Root; var; var = var->Next ) {
      int cmp = stricmp( node_name, var->name );
      if ( cmp == 0 ) {
        if ( i+1 == n_nodes ) {
          if ( var->type != Var_Node ) {
            node = var;
            return 0;
          } else {
            nl_error( 2, "Variable name conflicts with internal node" );
            return 1;
          }
        } else if ( var->type != Var_Node ) {
          nl_error( 2, "Variable node conflicts with existing variable" );
          return 1;
        }
        nl_assert( var->type == Var_Node );
        node = var;
        break;
      } else if ( cmp > 0 ) {
        sib = var;
      } else break;
    }
    // We get here if we either found a matching node or are ready to insert
    if ( node ) {
      parent = (RTG_Variable_Node *)node;
      node = NULL;
      sib = NULL;
    } else {
      if ( i+1 == n_nodes ) {
        last_node_text = node_name;
        return 0;
      }
      // Create a new internal node here
      RTG_Variable_Node *newnode =
        new RTG_Variable_Node(node_name, parent, sib);
      parent = newnode;
      sib = NULL;
    }
  }
  nl_error(4,"Should not have reached this point");
  return 1;
}

/* return true on overflow */
bool RTG_Variable::isnprint_path(char *&dest, int &n) {
  if (Parent) {
    if ( Parent->isnprint_path(dest, n) || n < 1 )
      return true;
    *dest++ = '/';
    --n;
  }
  int rc = snprintf(dest, n, "%s", name);
  if (rc < 0 || rc >= n) return true;
  dest += rc;
  n -= rc;
  return false;
}

bool RTG_Variable::snprint_path(char *dest, int n) {
  return isnprint_path(dest, n);
}

RTG_Variable_Data::RTG_Variable_Data( const char *name_in, RTG_Variable_Type type_in ) :
    RTG_Variable(name_in, type_in) {
  new_data_available = false;
  reload_required = false;
  ncols = 0;
}

RTG_Variable_Data::~RTG_Variable_Data() {
  // Need to delete any graphs or derivatives
  destroying = true;
  while (!graphs.empty())
    delete graphs.front();
  while (!derivatives.empty())
    delete(derivatives.front());
}

bool RTG_Variable_Data::check_for_updates() {
  if ( new_data_available ) {
    reload_required = true;
    new_data_available = false;
  }
  return reload_required;
}

/**
 * If reload_required, invokes reload_data() to do then work,
 * then marks all graphs as having new data and marks all
 * derivatives as requiring a reload. Ends by invoking reload()
 * on the next sibling.
 */
bool RTG_Variable_Data::reload() {
  if ( reload_required && reload_data() ) {
    reload_required = false;
    std::list<plot_graph*>::const_iterator pos;
    for (pos = graphs.begin(); pos != graphs.end(); ++pos) {
      plot_graph *graph = *pos;
      graph->new_data = true;
    }
    std::list<RTG_Variable_Derived *>::const_iterator dpos;
    for (dpos = derivatives.begin(); dpos != derivatives.end(); ++dpos) {
      (*dpos)->reload_required = true;
    }
    return true;
  }
  if (Next) return Next->reload();
  return false;
}

void RTG_Variable_Data::AddGraph(plot_graph *graph) {
  graphs.push_back(graph);
}

void RTG_Variable_Data::RemoveGraph(plot_graph *graph) {
  nl_assert(graph != NULL);
  nl_assert(!graphs.empty());
  graphs.remove(graph);
}
void RTG_Variable_Data::AddDerived(RTG_Variable_Derived *var) {
  derivatives.push_back(var);
}

void RTG_Variable_Data::RemoveDerived(RTG_Variable_Derived *var) {
  nl_assert(var != NULL);
  nl_assert(!derivatives.empty());
  derivatives.remove(var);
}

RTG_Variable_Data *RTG_Variable_Data::Derived_From() {
  return NULL;
}

RTG_Variable_Matrix::RTG_Variable_Matrix(const char *name_in, RTG_Variable_Type type_in) :
    RTG_Variable_Data(name_in, type_in) {
}

bool RTG_Variable_Matrix::get(unsigned r, unsigned c, scalar_t &X, scalar_t &Y) {
  if ( r >= nrows || c >= ncols ) return false;
  X = r;
  Y = data.mdata[c][r];
  return true;
}

vector_t RTG_Variable_Matrix::y_vector(unsigned col) {
  nl_assert(col < ncols && col < data.ncols);
  return data.mdata[col];
}

/**
 * What should I do if there is no data?
 * I can set the range to [0], but we really need to mark
 * the line as invisible.
 */
void RTG_Variable_Matrix::evaluate_range(unsigned col,
    RTG_Variable_Range &X, RTG_Variable_Range &Y) {
  unsigned r, r1;
  if (X.range_auto) {
    scalar_t y;
    get(0,col,X.min,y);
    get(nrows-1,col,X.max,y);
    X.range_required = false;
    X.range_is_current = true;
    X.range_is_empty = (nrows == 0);
  }
  xrow_range(X.min, X.max, r, r1 );
  if (r > r1)
    X.range_is_empty = true;
  if (Y.range_auto) {
    Y.clear();
    Y.range_required = false;
    Y.range_is_current = true;
    if (!X.range_is_empty && col < ncols) {
      vector_t Ydata = y_vector(col);
      Y.min = Y.max = Ydata[r++];
      for ( ; r <= r1; ++r ) {
        scalar_t V = Ydata[r];
        Y.update(V);
      }
      Y.range_is_empty = false;
    }
  }
}

char *RTG_Variable_MLF::default_path;

RTG_Variable_MLF::RTG_Variable_MLF( const char *name_in, RTG_Variable_Node *parent_in,
        RTG_Variable *sib ) : RTG_Variable_Matrix(name_in, Var_MLF) {
  char fbase[PATH_MAX];
  nl_assert(default_path != NULL);
  if (snprintf(fbase,PATH_MAX,"%s/%s", default_path, name_in) >= PATH_MAX) {
    nl_error(2,"Basename overflow in RTG_Variable_MLF");
    mlf = NULL;
  } else {
    mlf = mlf_init(3, 60, 0, fbase, "dat", NULL );
  }
  next_index = 0;
  update_ancestry(parent_in, sib);
}

/**
 * If range is empty returns i_min > i_max
 */
void RTG_Variable_MLF::xrow_range(scalar_t x_min, scalar_t x_max,
        unsigned &i_min, unsigned &i_max) {
  if (x_max < 0 || x_max < x_min || nrows == 0) {
    i_max = 0;
    i_min = 1;
  } else {
    if (x_min < 0) i_min = 0;
    else if (x_min >= nrows-1) i_min = nrows;
    else i_min = (unsigned)ceil(x_min);
    if (x_max >= nrows-1) i_max = nrows-1;
    else i_max = (unsigned)floor(x_max);
  }
}

bool RTG_Variable_MLF::reload_data() {
  mlf_set_index(mlf, next_index);
  FILE *fp = mlf_next_file(mlf);
  if ( fp != 0 ) {
    data.read_icos(fp);
    ncols = data.ncols;
    nrows = data.nrows;
    return true;
  }
  return false;
}

void RTG_Variable_MLF::new_index( unsigned long index ) {
  next_index = index;
  if (next_index > 0 && mlf != NULL && next_index != mlf->index
      && !new_data_available) {
    new_data_available = true;
    plot_obj::render_one();
    //if (!graphs.empty() && !derivatives.empty())
    //  plot_obj::setup_background();
  }
}

void RTG_Variable_MLF::set_default_path(const char *path) {
  nl_assert(path != NULL);
  if ( default_path ) free(default_path);
  default_path = strdup(path);
}

void RTG_Variable_MLF::Incoming( char *fullname, unsigned long index ) {
  RTG_Variable_Node *parent;
  RTG_Variable *sib, *node;
  RTG_Variable_MLF *mlf;
  char *lastnode_text;
  if ( Find_Insert( fullname, parent, sib, node, lastnode_text ) )
    return;
  if ( node ) {
    if ( node->type == Var_MLF ) {
      mlf = (RTG_Variable_MLF *)node;
      mlf->new_index(index);
    } else {
      nl_error( 2, "Variable %s is not an MLF variable", fullname );
      return;
    }
  } else {
    mlf = new RTG_Variable_MLF(lastnode_text, parent, sib);
    mlf->new_index(index);
    char fullname[80];
    if ( mlf->snprint_path(fullname, 80) )
      nl_error(1, "Overflow in nsprint_path()");
    else nl_error(0, "Data var '%s' created", fullname);
  }
}
