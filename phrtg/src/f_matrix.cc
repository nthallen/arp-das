/** \file f_matrix.cc
 */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "f_matrix.h"
#include "nortlib.h"

void f_matrix::init( unsigned rowsz, unsigned colsz ) {
  vdata = 0; mdata = 0;
  nrows = ncols = 0;
  maxrows = maxcols = 0;
  offset = 0;
  check( rowsz, colsz );
}

f_matrix::f_matrix( char *filename, int format ) {
  init( 0, 0 );
  switch (format ) {
    case FM_FMT_TEXT:
      read_text( filename, 128 );
      return;
    case FM_FMT_ICOS:
      read_icos( filename );
      return;
  }
  nl_error( 2, "Invalid or unsupported format" );
}

f_matrix::~f_matrix() {
  if ( mdata != 0 ) delete mdata;
  if ( vdata != 0 ) delete vdata;
  mdata = 0;
  vdata = 0;
  maxrows = maxcols = 0;
}

void f_matrix::read_text( char *filename, unsigned minrows ) {
  unsigned n_vars = 0;
  FILE *fp = fopen( filename, "r" );
  if ( fp == 0 ) {
    nl_error(2, "Unable to open file %s", filename );
    return;
  }
  for (;;) {
    char buf[MYBUFSIZE], *p, *ep;
    unsigned i;

    if ( fgets( buf, MYBUFSIZE, fp ) == 0 ) {
      fclose(fp);
      fp = 0;
      return;
    }
    if ( n_vars == 0 ) {
      for ( p = buf; ; p = ep ) {
        strtod( p, &ep );
        if ( p != ep ) n_vars++;
        else break;
      }
      check( minrows, n_vars );
      ncols = n_vars;
    }
    if ( nrows+1 >= maxrows ) check( maxrows+minrows, n_vars );
    for ( p = buf, i = 0; i < n_vars; i++ ) {
      mdata[i][nrows] = strtod( p, &ep );
      p = ep;
    }
    nrows++;
  }
}

void f_matrix::read_icos( FILE *fp ) {
  unsigned long dims[2];
  unsigned int i;
  if ( fp == 0 )
    return;
  if ( fread( dims, sizeof(unsigned long), 2, fp ) != 2 ) {
    nl_error( 2, "Error reading dimensions" );
    fclose(fp);
    return;
  }
  setsize(dims[0], dims[1], false);
  for ( i = 0; i < dims[1]; i++ ) {
    int ne = fread( mdata[i], sizeof(scalar_t), dims[0], fp );
    if ( ne != (int) dims[0] ) {
      nl_error(2,"Expected %d floats: fread returned %d", dims[0], ne );
      fclose(fp);
      return;
    }
  }
  fclose(fp);
}

void f_matrix::read_icos( const char *filename ) {
  FILE *fp = fopen(filename, "rb");
  read_icos(fp);
}

void f_matrix::check( unsigned rowsz, unsigned colsz, bool preserve ) {
  unsigned i;

  if ( rowsz > maxrows || colsz > maxcols ) {
    matrix_t newmdata = new vector_t[colsz];
    vector_t newvdata = new scalar_t[rowsz*colsz];
    if ( newmdata == 0 || newvdata == 0 )
      nl_error(3, "Out of memory in check" );
    for ( i = 0; i < colsz; i++ ) {
      newmdata[i] = &newvdata[i*rowsz];
    }
    if ( preserve && nrows != 0 && ncols != 0 ) {
      if ( nrows == maxrows && nrows == rowsz ) {
        memcpy( newvdata, vdata, nrows*ncols*sizeof(scalar_t) );
      } else {
        for ( i = 0; i < ncols; i++ ) {
          memcpy( newmdata[i], mdata[i], nrows*sizeof(scalar_t) );
        }
      }
    }
    if ( mdata != 0 ) delete mdata;
    if ( vdata != 0 ) delete vdata;
    vdata = newvdata;
    mdata = newmdata;
    maxrows = rowsz;
    maxcols = colsz;
  }
}

void f_matrix::setsize( unsigned nrows_in, unsigned ncols_in, bool preserve ) {
  check(nrows_in, ncols_in, preserve);
  nrows = nrows_in;
  ncols = ncols_in;
}
