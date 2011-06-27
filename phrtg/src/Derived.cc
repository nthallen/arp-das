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
  if ( Source->check_for_updates() )
    reload_required = true;
  return reload_required;
}

RTG_Variable_Data *RTG_Variable_Derived::Derived_From() {
  return Source;
}

bool RTG_Variable_Derived::reload_data() {
  if (Source->reload_required) return false; // wait till the new data is in
  ncols = Source->ncols;
  derive_required.resize(ncols);
  for (unsigned i = 0; i < ncols; i++)
    derive_required[i] = true;
  return true;
}

bool RTG_Variable_Derived::get(unsigned r, unsigned c, scalar_t &X, scalar_t &Y) {
  if ( r >= nrows || c >= ncols ) return false;
  if ( derive_required[c] ) derive(c);
  X = data.mdata[0][r];
  Y = data.mdata[c+1][r];
  return true;
}

vector_t RTG_Variable_Derived::y_vector(unsigned col) {
  nl_assert(col < ncols && col+1 < data.ncols);
  if ( derive_required[col] ) derive(col);
  return data.mdata[col+1];
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
  if ( RTG_Variable_Derived::reload_data() ) {
    Source->xrow_range(x_min, x_max, i_min, i_max);
    nrows = i_max >= i_min ? i_max - i_min + 1 : 0;
    data.setsize(nrows, ncols+1, false);
    return ncols > 0 && nrows > 0;
  }
  return false;
}

void RTG_Variable_Detrend::derive(unsigned c) {
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
  derive_required[c] = false;
}

/**
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

/**
 * Creates a variable named:
 *   DT(<var>,m,M)
 * This functionality could be contained in the constructor for
 * RTG_Variable_Detrend, but this approach allows us to return
 * NULL on various non-fatal error conditions before the
 * actual variable is created.
 * @return New detrended variable or NULL on error.
 */
RTG_Variable_Detrend *RTG_Variable_Detrend::Create( RTG_Variable_Data *src,
    scalar_t min, scalar_t max ) {
  RTG_Variable_Detrend *dt = NULL;
  RTG_Variable_Node *parent;
  RTG_Variable *sib, *node;
  const char *lastnode_text;
  char fullname[MAX_VAR_LENGTH];
  unsigned i_min, i_max;
  int n, rc;

  src->xrow_range(min, max, i_min, i_max);
  if ( src->Parent != NULL ) {
    if ( src->Parent->snprint_path( fullname, MAX_VAR_LENGTH) ) {
      nl_error(2, "Path overflow in Detrend::Create");
      return NULL;
    }
    n = strlen(fullname);
    fullname[n++] = '/';
  } else n = 0;

  rc = snprintf(fullname+n, MAX_VAR_LENGTH-n, "DT(%s,%u,%u)",
    src->name, i_min, i_max);
  if ( n + rc >= MAX_VAR_LENGTH ) {
    nl_error(2, "Path overflow in Detrend::Create [2]");
    return NULL;
  }
  if ( Find_Insert( fullname, parent, sib, node, lastnode_text ) )
    return NULL;
  if ( node ) {
    if ( node->type == Var_Detrend ) {
      dt = (RTG_Variable_Detrend *)node;
    } else {
      nl_error( 1, "Variable %s is not a detrend variable", fullname );
    }
  } else {
    dt = new RTG_Variable_Detrend(src, lastnode_text, parent, sib, min, max);
  }
  return dt;
}


RTG_Variable_Invert::RTG_Variable_Invert(RTG_Variable_Data *src,
      const char *name_in, RTG_Variable_Node *parent_in,
      RTG_Variable *sib )
    : RTG_Variable_Derived(src, name_in, Var_Invert) {
  update_ancestry(parent_in, sib);
  reload_required = true;
}

bool RTG_Variable_Invert::reload_data() {
  if ( RTG_Variable_Derived::reload_data() ) {
    nrows = Source->nrows;
    data.setsize(nrows, ncols+1, false);
    return ncols > 0 && nrows > 0;
  }
  return false;
}

void RTG_Variable_Invert::xrow_range(scalar_t x_min, scalar_t x_max,
            unsigned &i_min, unsigned &i_max) {
  Source->xrow_range(x_min, x_max, i_min, i_max);
}

void RTG_Variable_Invert::derive(unsigned col) {
  vector_t x = data.mdata[0];
  vector_t v = data.mdata[col+1];
  for (unsigned i = 0; i < nrows; i++) {
    scalar_t y;
    Source->get(i, col, x[i], y);
    v[i] = -y;
  }
  derive_required[col] = false;
}

/**
 * Very similar to RTG_Variable_Detrend::Create, but does not
 * use x range
 */
RTG_Variable_Invert *RTG_Variable_Invert::Create( RTG_Variable_Data *src ) {
  RTG_Variable_Invert *inv = NULL;
  RTG_Variable_Node *parent;
  RTG_Variable *sib, *node;
  const char *lastnode_text;
  char fullname[MAX_VAR_LENGTH];
  int n, rc;

  if ( src->Parent != NULL ) {
    if ( src->Parent->snprint_path( fullname, MAX_VAR_LENGTH) ) {
      nl_error(2, "Path overflow in Invert::Create");
      return NULL;
    }
    n = strlen(fullname);
    fullname[n++] = '/';
  } else n = 0;

  rc = snprintf(fullname+n, MAX_VAR_LENGTH-n, "INV(%s)", src->name);
  if ( n + rc >= MAX_VAR_LENGTH ) {
    nl_error(2, "Path overflow in Invert::Create [2]");
    return NULL;
  }
  if ( Find_Insert( fullname, parent, sib, node, lastnode_text ) )
    return NULL;
  if ( node ) {
    if ( node->type == Var_Invert ) {
      inv = (RTG_Variable_Invert *)node;
    } else {
      nl_error( 1, "Variable %s is not an invert variable", fullname );
    }
  } else {
    inv = new RTG_Variable_Invert(src, lastnode_text, parent, sib);
  }
  return inv;
}

RTG_Variable_FFT::RTG_Variable_FFT(RTG_Variable_Data *src,
    const char *name_in, RTG_Variable_Node *parent_in, RTG_Variable *sib,
    scalar_t min, scalar_t max)
    : RTG_Variable_Derived(src, name_in, Var_FFT) {
  x_min = min;
  x_max = max;
  Ni = 0;
  update_ancestry(parent_in, sib);
  reload_required = true;
  // Delay setting the size and planning the transform
  // until we know the source variable has data,
  // i.e. on reload_data().
}

RTG_Variable_FFT *RTG_Variable_FFT::Create(RTG_Variable_Data *src,
	scalar_t min, scalar_t max) {
  RTG_Variable_FFT *fft = NULL;
  RTG_Variable_Node *parent;
  RTG_Variable *sib, *node;
  const char *lastnode_text;
  char fullname[MAX_VAR_LENGTH];
  unsigned i_min, i_max;
  int n, rc;

  src->xrow_range(min, max, i_min, i_max);
  if ( src->Parent != NULL ) {
    if ( src->Parent->snprint_path( fullname, MAX_VAR_LENGTH) ) {
      nl_error(2, "Path overflow in FFT::Create");
      return NULL;
    }
    n = strlen(fullname);
    fullname[n++] = '/';
  } else n = 0;

  rc = snprintf(fullname+n, MAX_VAR_LENGTH-n, "FFT(%s,%u,%u)",
    src->name, i_min, i_max);
  if ( n + rc >= MAX_VAR_LENGTH ) {
    nl_error(2, "Path overflow in FFT::Create [2]");
    return NULL;
  }
  if ( Find_Insert( fullname, parent, sib, node, lastnode_text ) )
    return NULL;
  if ( node ) {
    if ( node->type == Var_FFT ) {
      fft = (RTG_Variable_FFT *)node;
    } else {
      nl_error( 1, "Variable %s is not an fft variable", fullname );
    }
  } else {
    fft = new RTG_Variable_FFT(src, lastnode_text, parent, sib, min, max);
  }
  return fft;
}

void RTG_Variable_FFT::AddGraph(plot_graph *graph) {
  return; // Just defeat it.
  // ###
  // Need to derive a PSD and graph that instead
}

bool RTG_Variable_FFT::reload_data() {
  if ( RTG_Variable_Derived::reload_data() ) {
    unsigned new_nr;
    Source->xrow_range(x_min, x_max, i_min, i_max);
    Ni = i_max >= i_min ? i_max - i_min + 1 : 0;
    new_nr = 1 + Ni/2;
    if (new_nr != nrows) {
      nrows = new_nr;
      data.setsize(nrows*2, ncols+1, false);
      plans.resize(ncols);
    }
    return ncols > 0 && nrows > 0;
  }
  return false;
}

void RTG_Variable_FFT::derive(unsigned col) {
  vector_t iv = Source->y_vector(col);
  vector_t ov = data.mdata[col+1];
  plans[col].fft(iv+i_min, ov, Ni);
  derive_required[col] = false;
}

/**
 * I am using the same logic as RTG_Variable_MLF,
 * but I rescale the X values to go from 0 to 1
 * instead, where 1 is the Nyquist frequency.
 */
void RTG_Variable_FFT::xrow_range(scalar_t x_min, scalar_t x_max,
        unsigned &i_min, unsigned &i_max) {
  x_min *= nrows-1;
  x_max *= nrows-1;
  if (x_max < 0 || x_max < x_min || nrows == 0 || x_min > nrows) {
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

FFT_Plan::FFT_Plan() {
  ivec = ovec = NULL;
  N = 0;
  P = NULL;
}

FFT_Plan::~FFT_Plan() {
  if (P != NULL)
    fftwf_destroy_plan(P);
}

void FFT_Plan::fft(scalar_t *iv, scalar_t *ov, int Npts ) {
  if ( ivec != iv || ovec != ov || Npts != N ) {
    ivec = iv;
    ovec = ov;
    N = Npts;
    if ( P != NULL ) fftwf_destroy_plan(P);
    P = fftwf_plan_dft_r2c_1d(N,ivec,(fftwf_complex *)ovec,FFTW_ESTIMATE);
  }
  fftwf_execute(P);
}

RTG_Variable_PSD::RTG_Variable_PSD(RTG_Variable_Data *src, const char *name_in,
    RTG_Variable_Node *parent_in, RTG_Variable *sib)
    : RTG_Variable_Derived(src, name_in, Var_FFT_PSD) {
  update_ancestry(parent_in, sib);
  reload_required = true;
}

bool RTG_Variable_PSD::reload_data() {
  if ( RTG_Variable_Derived::reload_data() ) {
    unsigned i;
    nrows = Source->nrows;
    data.setsize(nrows, ncols+1, false);
    for (i = 0; i < nrows; ++i ) {
      data.mdata[0][i] = i/(nrows-1.0);
    }
    return ncols > 0 && nrows > 0;
  }
  return false;
}

void RTG_Variable_PSD::derive(unsigned col) {
  scalar_t *iv = Source->y_vector(col);
  scalar_t *ov = data.mdata[col+1];
  unsigned i;
  for ( i = 0; i < nrows; ++i ) {
    ov[i] = sqrtf(iv[i*2]*iv[i*2] + iv[i*2+1]*iv[i*2+1]);
  }
  derive_required[col] = false;
}

void RTG_Variable_PSD::xrow_range(scalar_t x_min, scalar_t x_max,
	unsigned &i_min, unsigned &i_max) {
  Source->xrow_range(x_min, x_max, i_min, i_max);
}

RTG_Variable_PSD *RTG_Variable_PSD::Create( RTG_Variable_Data *src,
	scalar_t min, scalar_t max ) {
  RTG_Variable_FFT *fft = NULL;
  RTG_Variable_PSD *psd = NULL;
  RTG_Variable_Node *parent;
  RTG_Variable *sib, *node;
  const char *lastnode_text;
  char fullname[MAX_VAR_LENGTH];
  unsigned i_min, i_max;
  int n, rc;

  fft = RTG_Variable_FFT::Create(src, min, max);
  src->xrow_range(min, max, i_min, i_max);
  if ( src->Parent != NULL ) {
    if ( src->Parent->snprint_path( fullname, MAX_VAR_LENGTH) ) {
      nl_error(2, "Path overflow in PSD::Create");
      return NULL;
    }
    n = strlen(fullname);
    fullname[n++] = '/';
  } else n = 0;

  rc = snprintf(fullname+n, MAX_VAR_LENGTH-n, "PSD(%s,%u,%u)",
    src->name, i_min, i_max);
  if ( n + rc >= MAX_VAR_LENGTH ) {
    nl_error(2, "Path overflow in PSD::Create [2]");
    return NULL;
  }
  if ( Find_Insert( fullname, parent, sib, node, lastnode_text ) )
    return NULL;
  if ( node ) {
    if ( node->type == Var_FFT_PSD ) {
      psd = (RTG_Variable_PSD *)node;
    } else {
      nl_error( 1, "Variable %s is not a psd variable", fullname );
    }
  } else {
    psd = new RTG_Variable_PSD(fft, lastnode_text, parent, sib);
  }
  return psd;
}
