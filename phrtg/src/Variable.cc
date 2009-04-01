/* Variable.cc
  Support routines for the Variable tab
  */
#include <ctype.h>
#include "ablibs.h"
#include "phrtg.h"
#include "abimport.h"
#include "nl_assert.h"

RTG_Variable *RTG_Variable::Root;
RTG_Variable *Current::Variable;

RTG_Variable::RTG_Variable(const char *name_in, RTG_Variable_Type type_in) {
  nl_assert(name_in != NULL);
  Parent = NULL;
  Next = NULL;
  name = strdup(name_in);
  type = type_in;
  TreeItem = PtTreeAllocItem(ABW_Variables_Tab, name, -1, -1);
  TreeItem->data = (void *)this;
}

RTG_Variable::~RTG_Variable() {
	nl_error(2, "~RTG_Variable");
}

void RTG_Variable::AddSibling(RTG_Variable *newsib) {
  nl_assert(newsib != NULL);
  newsib->Next = this->Next;
  this->Next = newsib;
  newsib->Parent = this->Parent;
  PtTreeAddAfter(ABW_Variables_Tab, newsib->TreeItem, this->TreeItem);
}

void RTG_Variable::AddGraph(plot_data *graph) {
  graphs.push_back(graph);
}

void RTG_Variable::RemoveGraph(plot_data *graph) {
  nl_assert(graph != NULL);
  nl_assert(!graphs.empty());
  graphs.remove(graph);
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

// This is the Pt_CB_TREE_SELECTION callback for the Variables Tab
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
	  Current::Variable = var;
	  nl_error( 0, "Variable %s selected", var->name );
	}
  }
  return Pt_CONTINUE;
}

RTG_Variable_Node::RTG_Variable_Node(const char *name_in) : RTG_Variable(name_in, Var_Node) {
  First = NULL;
}

void RTG_Variable_Node::Add_Child(RTG_Variable *child) {
  nl_assert(child != NULL);
  child->Next = this->First; // just to test compiler, should be child->Next
  this->First = child;
  child->Parent = this;
  PtTreeAddFirst( ABW_Variables_Tab, child->TreeItem, this->TreeItem );
}

/*
 * I should probably impose some arbitrary limits on name length
 * Overall length should be less than 80
 * Individual node length should not exceed 40
 * Number of nodes in a name should not exceed 6
 * Return non-zero on error
*/
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
      RTG_Variable_Node *newnode = new RTG_Variable_Node(node_name);
      newnode->update_ancestry( parent, sib );
      parent = newnode;
      sib = NULL;
    }
  }
  nl_error(4,"Should not have reached this point");
  return 1;
}

RTG_Variable_Data::RTG_Variable_Data( const char *name_in, RTG_Variable_Type type_in ) :
  RTG_Variable(name_in, type_in) {
}

char *RTG_Variable_MLF::default_path;

RTG_Variable_MLF::RTG_Variable_MLF( const char *name_in ) :
    RTG_Variable_Data(name_in, Var_MLF) {
  nl_assert(default_path != NULL);
  path = strdup(default_path);
}

void RTG_Variable_MLF::Incoming( char *fullname, int index ) {
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
    mlf = new RTG_Variable_MLF(lastnode_text);
    mlf->update_ancestry( parent, sib );
    mlf->new_index(index);
  }
}

void RTG_Variable_MLF::set_default_path(const char *path) {
  nl_assert(path != NULL);
  if ( default_path ) free(default_path);
  default_path = strdup(path);
}

void RTG_Variable_MLF::new_index( int index ) {
  // ###
}
