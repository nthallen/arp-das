/** \file trend.cc
 * Support for trending graphs
 */
#include "ablibs.h"
#include "phrtg.h"
#include "nortlib.h"

#ifdef JUHLH
    /** The number of Y columns */
    int n_cols;
    double x_max, x_min, x_epoch, span;
    /** Guarantees monotonicity, column coherency */
    void push( int nc, double X, double *Y );
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
    bool get(unsigned r, unsigned c, scalar_t &X, scalar_t &Y, double epoch);
#endif

trend_queue::trend_queue() {
  n_cols = 0;
  x_max = x_min = x_epoch = span = 0.;
}

/* Guarantees monotonicity, column coherency */
void trend_queue::push( int nc, double X, double *Y ) {
  int i;
  
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

void trend_queue::flush( trend_queue &dest ) {
  while ( !empty() ) {
    std::vector<double> Y;
    double X;
    int i;
    
    nl_assert( size() > n_cols );
    X = pop_front() + x_epoch;
    for ( i = 0; i < n_cols; ++i ) {
      Y.push_back(pop_front());
    }
    dest.push(n_cols, X, &Y[0]);
  }
}

/**
 * Retire oldest points, but keep at least MINPOINTS
 * and any points within span of the most recent.
 */
void trend_queue::flush() {
  int n;
  double cutoff;
  nl_assert(span >= 0.);
  n = n_rows() - MINPOINTS;
  cutoff = x_max - span;
  while ( n > 0 && x_min < cutoff ) {
    x_min = *(erase(0, n_cols));
    --n;
  }
}

int trend_queue::n_rows() {
  return size()/n_cols;
}

    /**
     * Sets X and Y to the values for the specified row and column.
     * The X value is adjusted to be relative to the specified epoch.
     * @return true on success, false if indices are out of range
     */
bool trend_queue::get(unsigned r, unsigned c, scalar_t &X, scalar_t &Y, double epoch) {
}

void RTG_Variable_Trend::Incoming(const char *cmd) {
  nl_error(0, "RTG_Variable_Trend::Incoming: %s", cmd);
  //### parse input
  //###
  pending.push(nc, X, &Y[0]);
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
    data.flush();
    return true;
  }
  return false;
}

bool RTG_Variable_Trend::get(unsigned r, unsigned c, scalar_t &X, scalar_t &Y) {
}

void RTG_Variable_Trend::evaluate_range(unsigned col, RTG_Range &X,
    RTG_Range &Y);
void RTG_Variable_Trend::xrow_range(scalar_t x_min, scalar_t x_max,
    unsigned &i_min, unsigned &i_max);
vector_t RTG_Variable_Trend::y_vector(unsigned col);
