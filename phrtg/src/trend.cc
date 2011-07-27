/** \file trend.cc
 * Support for trending graphs
 */
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include "ablibs.h"
#include "phrtg.h"
#include "nortlib.h"
#include "nl_assert.h"

trend_queue::trend_queue() {
  n_cols = 0;
  x_max = x_min = x_epoch = span = 0.;
}

/* Guarantees monotonicity, column coherency */
void trend_queue::push( unsigned nc, double X, double *Y ) {
  unsigned i;
  
  if (!empty()) {
    if ( nc != n_cols || X - x_epoch <= x_max ) {
      clear();
      n_cols = nc;
    }
  }
  if ( empty() ) {
    x_epoch = X;
    x_max = x_min = 0.;
    n_cols = nc;
  }
  X -= x_epoch;
  push_back(X);
  if ( X > x_max ) x_max = X;
  else if ( X < x_min ) x_min = X;
  for ( i = 0; i < n_cols; ++i ) {
    push_back(Y[i]);
  }
}

/**
 * Copy data from pending queue to destination data
 * queue. dest.n_cols may be updated as a result.
 */
void trend_queue::flush( trend_queue &dest ) {
  while ( !empty() ) {
    std::vector<double> Y;
    double X;
    unsigned i;
    
    nl_assert( size() > n_cols );
    X = front() + x_epoch;
    pop_front();
    for ( i = 0; i < n_cols; ++i ) {
      Y.push_back(front());
      pop_front();
    }
    dest.push(n_cols, X, &Y[0]);
  }
}

/**
 * Retire oldest points, but keep at least MINPOINTS
 * and any points within span of the most recent.
 * Does not change n_cols.
 */
void trend_queue::flush() {
  int n;
  double cutoff;
  nl_assert(span >= 0.);
  n = n_rows() - MINPOINTS;
  cutoff = x_max - span;
  while ( n > 0 && x_min < cutoff ) {
    x_min = *(erase(begin(), begin()+n_cols));
    --n;
  }
}

unsigned trend_queue::n_rows() {
  return size()/(n_cols+1);
}

    /**
     * Sets X and Y to the values for the specified row and column.
     * The X value is adjusted to be relative to the specified epoch.
     * @return true on success, false if indices are out of range
     */
bool trend_queue::get(unsigned r, unsigned c, double &X, double &Y) {
  if ( r >= n_rows() || c >= n_cols ) return false;
  X = (*this)[r*(n_cols+1)] + x_epoch;
  Y = (*this)[r*(n_cols+1) + c + 1];
  return true;
}

double trend_queue::find_x(double x_in) {
  unsigned low = 0, high = n_rows();
  double lowV = x_min, highV = x_max;
  nl_assert(high > 0);
  --high;
  for (;;) {
    double guess, guessV, Y;
    unsigned gi;
    if ( x_in <= lowV ) return ((double)low );
    if ( x_in >= highV ) return ((double)high);
    guess = low + (high-low) * (x_in - lowV)/(highV-lowV);
    if ( high-low <= 1 ) return guess;
    gi = floor(guess+.5);
    if ( gi == low ) ++gi;
    if ( gi == high ) --gi;
    get( gi, 0, guessV, Y );
    if ( guessV == x_in ) return((double)gi);
    if ( x_in < guessV ) {
      high = gi;
      highV = guessV;
    } else {
      low = gi;
      lowV = guessV;
    }
  }
}

RTG_Variable_Trend::RTG_Variable_Trend(const char *name_in, RTG_Variable_Node *parent_in, RTG_Variable *sib)
    : RTG_Variable_Data(name_in, Var_Trend) {
  has_y_vector = false;
  update_ancestry(parent_in, sib);
}

/**
 * Incoming command string consists of:
 *   Name X Y+
 */
void RTG_Variable_Trend::Incoming(const char *cmd) {
  RTG_Variable_Node *parent;
  RTG_Variable *sib, *node;
  RTG_Variable_Trend *trend;
  const char *lastnode_text;
  std::string varname;
  const char *p;
  char *q;
  unsigned i;
  double X;
  std::vector<double> Y;
  
  if ( cmd == NULL ) return;
  for ( p = cmd; isspace(*p); ++p );
  if ( *p == '\0' ) return;
  for ( i = 1; p[i] != '\0' && ! isspace(p[i]); ++i);
  varname.assign(p,i);
  if ( Find_Insert( varname.c_str(), parent, sib, node, lastnode_text ) )
    return; // bad name
  p += i;

  if ( node ) {
    if ( node->type == Var_Trend ) {
      trend = (RTG_Variable_Trend *)node;
    } else {
      nl_error( 2, "Variable %s is not a Trend variable", varname.c_str() );
      return;
    }
  } else {
    trend = new RTG_Variable_Trend(lastnode_text, parent, sib);
    char fullname[80];
    if ( trend->snprint_path(fullname, 80) )
      nl_error(1, "Overflow in nsprint_path()");
    else nl_error(0, "Trend var '%s' created", fullname);
  }
  errno = 0;
  X = strtod( p, &q );
  if ( errno != 0 ) {
    nl_error( 2, "Error parsing trend string X: %s", cmd );
    return;
  }
  while (q != NULL && *q != '\0') {
    double YV;
    p = q;
    errno = 0;
    YV = strtod( p, &q );
    if ( p == q || (errno != 0 && errno != ERANGE) )
      break;
    Y.push_back(YV);
  }
  trend->pending.push(Y.size(), X, &Y[0]);
  trend->new_data_available = true;
  trend->pending.flush(); // Don't allow it to grow without limit
  plot_obj::render_one();
}

bool RTG_Variable_Trend::reload_data() {
  double span = 0.;
  std::list<plot_graph*>::const_iterator pos;

  for (pos = graphs.begin(); pos != graphs.end(); ++pos) {
    double grspan;
    plot_graph *graph = *pos;
    grspan = graph->parent->X.limits.span;
    if ( grspan > span ) span = grspan;
  }
  pending.span = data.span = span;
  if ( ! pending.empty() ) {
    pending.flush(data);
    ncols = data.n_cols;
    data.flush();
    nrows = data.n_rows();
    return true;
  }
  return false;
}

bool RTG_Variable_Trend::get(unsigned r, unsigned c, double &X, double &Y) {
  return data.get(r, c, X, Y);
}

/**
 * This implementation is very similar to the one for RTG_Variable_Matrix,
 * but it does not rely on the y_vector() method. One could argue that
 * this should be promoted to an implementation for RTG_Variable_Data.
 * That would not eliminate the need for y_vector(), though.
 */
void RTG_Variable_Trend::evaluate_range(unsigned col, RTG_Range &X,
    RTG_Range &Y) {
  unsigned r, r1;
  if (X.range_required) {
    X.range_required = false;
    if (nrows == 0) {
      X.range_is_empty = true;
    } else {
      double y;
      X.range_is_empty = false;
      get(0,col,X.min,y);
      get(nrows-1,col,X.max,y);
    }
    X.range_is_current = true;
    X.range_updated = true;
  }
  xrow_range(X.min, X.max, r, r1 );
  if (r > r1)
    X.range_is_empty = true;
  if (Y.range_required) {
    Y.clear();
    Y.range_required = false;
    if (!X.range_is_empty && col < ncols) {
      for ( ; r <= r1; ++r ) {
        double XV, YV;
        get(r,col, XV, YV);
        Y.update(YV);
      }
    }
    Y.range_is_current = true;
    Y.range_updated = true;
  }
}

/* The other main implementation at this point is RTG_Variable_MLF,
 * where the X axis is implicit, so it's really simple. Here we
 * have an explicit X axis, which we know to be monotonic and
 * usually linear.
 */
void RTG_Variable_Trend::xrow_range(double x_min, double x_max,
    unsigned &i_min, unsigned &i_max) {
  if ( data.n_rows() == 0 ||
       x_max < x_min ||
       x_max < data.x_min + data.x_epoch ||
       x_min > data.x_max + data.x_epoch ) {
    i_max = 0;
    i_min = 1;
  } else {
    i_min = (unsigned)floor(data.find_x(x_min));
    i_max = (unsigned)ceil(data.find_x(x_max));
  }
}

/**
 * We won't support the y_vector operation for now.
 * @return NULL
 */
vector_t RTG_Variable_Trend::y_vector(unsigned col) {
  return NULL;
}
