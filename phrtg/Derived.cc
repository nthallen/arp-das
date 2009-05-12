/* Derived.cc
 * Handles RTG_Variable_Detrend and maybe RTG_Variable_FFT
 */
#include <ctype.h>
#include <math.h>
#include "ablibs.h"
#include "phrtg.h"
#include "abimport.h"
#include "nl_assert.h"

RTG_Variable_Derived::RTG_Variable_Derived(RTG_Variable_Data *src,
        const char *name_in, RTG_Variable_Type type_in)
    : RTG_Variable_Matrix(name_in, type_in) {
  Source = src;
}

void RTG_Variable_Derived::RemoveGraph(plot_data *graph) {
  RTG_Variable_Data::RemoveGraph(graph);
  if (graphs.empty() && derivatives.empty())
    delete this;
}

void RTG_Variable_Derived::RemoveDerived(RTG_Variable_Derived *var) {
  RTG_Variable_Data::RemoveDerived(var);
  if (derivatives.empty() && graphs.empty())
    delete this;
}

/* Returns true if Source has new data */
bool RTG_Variable_Derived::reload_data() {
  return Source->reload_data();
}

RTG_Variable_Data *RTG_Variable_Derived::Derived_From() {
  return Source;
}

RTG_Variable_Detrend::RTG_Variable_Detrend(RTG_Variable_Data *src,
    const char *name_in, RTG_Variable_Node *parent_in, RTG_Variable *sib,
    scalar_t min, scalar_t max)
    : RTG_Variable_Derived(src, name_in, Var_Detrend) {
  x_min = min;
  x_max = max;
  update_ancestry(parent_in, sib);
}

bool RTG_Variable_Detrend::reload_data() {
  // calls RTG_Variable_Derived::reload_data();
  if (RTG_Variable_Derived::reload_data()) {
    // And then deals with it!
    // Need to make sure we have as much space as src,
    // Then check endpoints and do the detrend operation.
    return true;
  }
  return false;
}

RTG_Variable_Detrend *RTG_Variable_Detrend::Create( RTG_Variable_Data *src,
        unsigned min, unsigned max ) {
  RTG_Variable_Detrend *dt;
  RTG_Variable_Node *parent;
  RTG_Variable *sib, *node;
  char *lastnode_text;
  char fullname[80];
  strcpy(fullname, "Detrend");
  int n = strlen(fullname);
  fullname[n++] = '/';
  if ( src->snprint_path(fullname+n, 80-n) ) {
    nl_error(2, "Path overflow in Detrend::Create");
    return NULL;
  }
  n = strlen(fullname);
  int rc = snprintf(fullname+n, 80-n, "/X%d_%d", min, max);
  if (n+rc >= 80) {
    nl_error(2, "Path overflow in Detrend::Create [2]");
    return NULL;
  }
  if ( Find_Insert( fullname, parent, sib, node, lastnode_text ) )
    return NULL;
  if ( node ) {
    if ( node->type == Var_Detrend ) {
      dt = (RTG_Variable_Detrend *)node;
    } else {
      nl_error( 2, "Variable %s is not a detrend variable", fullname );
      return NULL;
    }
  } else {
    dt = new RTG_Variable_Detrend(src, lastnode_text, parent, sib, min, max);
  }
  return dt;
}
