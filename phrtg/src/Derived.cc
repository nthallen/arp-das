/** \file Derived.cc
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
  Source->AddDerived(this);
}

RTG_Variable_Derived::~RTG_Variable_Derived() {
  nl_assert(Source != NULL);
  Source->RemoveDerived(this);
}

void RTG_Variable_Derived::RemoveGraph(plot_graph *graph) {
  RTG_Variable_Data::RemoveGraph(graph);
  if (graphs.empty() && derivatives.empty())
    delete this;
}

void RTG_Variable_Derived::RemoveDerived(RTG_Variable_Derived *var) {
  RTG_Variable_Data::RemoveDerived(var);
  if (derivatives.empty() && graphs.empty())
    delete this;
}

bool RTG_Variable_Derived::check_for_updates() {
  return Source->check_for_updates() || reload_required;
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
  reload_required = true;
}

bool RTG_Variable_Detrend::reload_data() {
  if (Source->reload_required) return false; // wait till the new data is in
  ncols = Source->ncols;
  detrend_required.resize(ncols);
  for (unsigned i = 0; i < ncols; i++)
    detrend_required[i] = true;
  Source->xrow_range(x_min, x_max, i_min, i_max);
  nrows = i_max >= i_min ? i_max - i_min + 1 : 0;
  data.setsize(nrows, ncols+1, false);
  return ncols > 0 && nrows > 0;
}

bool RTG_Variable_Detrend::get(unsigned r, unsigned c, scalar_t &X, scalar_t &Y) {
  if ( r >= nrows || c >= ncols ) return false;
  if ( detrend_required[c] ) detrend(c);
  X = data.mdata[0][r];
  Y = data.mdata[c+1][r];
  return true;
}

void RTG_Variable_Detrend::detrend(unsigned c) {
  scalar_t xx, y0, y1, m;
  vector_t x = data.mdata[0];
  vector_t v = data.mdata[c+1];
  if (!Source->get(i_min, c, xx, y0) ||
      !Source->get(i_max, c, xx, y1) )
    nl_error(4, "Error in Source->get from Detrend");
  m = nrows > 1 ? (y1-y0)/(nrows-1) : 0;
  for (unsigned i = 0; i < nrows; i++) {
    scalar_t y;
    Source->get(i_min+i, c, x[i], y);
    v[i] = y - y0;
    y0 += m;
  }
  detrend_required[c] = false;
}

vector_t RTG_Variable_Detrend::y_vector(unsigned col) {
  nl_assert(col < ncols && col+1 < data.ncols);
  if ( detrend_required[col] ) detrend(col);
  return data.mdata[col+1];
}

/*
 * Detrend maps directly onto a subset of the Source range, so
 * we can call the source's xrow_range and offset by i_min.
 */
void RTG_Variable_Detrend::xrow_range(scalar_t x_min, scalar_t x_max,
        unsigned &i_min, unsigned &i_max) {
  Source->xrow_range(x_min, x_max, i_min, i_max);
  if ( i_max > i_min ) {
    if (i_max > this->i_max) i_max = this->i_max;
    if (i_max < this->i_min || i_min > this->i_max) {
      i_min = 1;
      i_max = 0;
    } else {
      i_max -= this->i_min;
      if (i_min <= this->i_min ) i_min = 0;
      else i_min -= this->i_min;
    }
  }
}

/* Creates a variable named:
 *   /Detrend/<var>/Xn
 * Where Xn is X1, X2, ... and corresponds to possible X ranges
 */
RTG_Variable_Detrend *RTG_Variable_Detrend::Create( RTG_Variable_Data *src,
    scalar_t min, scalar_t max ) {
  RTG_Variable_Detrend *dt = NULL;
  RTG_Variable_Node *parent;
  RTG_Variable *sib, *node;
  char *lastnode_text;
  char fullname[80];
  strcpy(fullname, "Detrend/");
  int n = strlen(fullname);
  if ( src->snprint_path(fullname+n, 80-n) ) {
    nl_error(2, "Path overflow in Detrend::Create");
    return NULL;
  }
  n = strlen(fullname);
  for ( unsigned Xi = 0; dt == NULL; ++Xi) {
    int rc = snprintf(fullname+n, 80-n, "/X%u", Xi);
    if (n+rc >= 80) {
      nl_error(2, "Path overflow in Detrend::Create [2]");
      return NULL;
    }
    if ( Find_Insert( fullname, parent, sib, node, lastnode_text ) )
      return NULL;
    if ( node ) {
      if ( node->type == Var_Detrend ) {
        dt = (RTG_Variable_Detrend *)node;
        if (dt->x_min != min || dt->x_max != max)
          dt = NULL;
      } else {
        nl_error( 1, "Variable %s is not a detrend variable", fullname );
      }
    } else {
      dt = new RTG_Variable_Detrend(src, lastnode_text, parent, sib, min, max);
    }
  }
  return dt;
}
