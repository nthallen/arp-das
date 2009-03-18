#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "f_matrix.h"
#include "nortlib.h"

f_matrix::f_matrix( int rowsz, int colsz ) {
  vdata = 0; mdata = 0;
  nrows = ncols = 0;
  maxrows = maxcols = 0;
  offset = 0;
  check( rowsz, colsz );
}

f_matrix::f_matrix( char *filename, int format ) {
  f_matrix( 0, 0 );
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

void f_matrix::read_text( char *filename, int minrows ) {
  int n_vars = 0;
  FILE *fp = fopen( filename, "r" );
  if ( fp == 0 ) {
    nl_error(2, "Unable to open file %s", filename );
    return;
  }
  for (;;) {
    char buf[MYBUFSIZE], *p, *ep;
    int i;

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

void f_matrix::read_icos( char *filename ) {
  unsigned long dims[2];
  unsigned int i;
  FILE *fp = fopen( filename, "r" );
  if ( fp == 0 ) {
    nl_error( 2, "Unable to open file %s", filename );
    return;
  }
  if ( fread( dims, sizeof(unsigned long), 2, fp ) != 2 ) {
    nl_error( 2, "Error reading dimensions" );
    fclose(fp);
    return;
  }
  check(dims[0], dims[1]);
  for ( i = 0; i < dims[1]; i++ ) {
    int ne = fread( mdata[i], sizeof(scalar_t), dims[0], fp );
    if ( ne != (int) dims[0] ) {
      nl_error(2,"Expected %d floats: fread returned %d", dims[0], ne );
      fclose(fp);
      return;
    }
  }
  nrows = dims[0];
  ncols = dims[1];
  fclose(fp);
}

void f_matrix::check( int rowsz, int colsz ) {
  int i;

  if ( rowsz > maxrows || colsz > maxcols ) {
    matrix_t newmdata = new vector_t[colsz];
    vector_t newvdata = new scalar_t[rowsz*colsz];
    if ( newmdata == 0 || newvdata == 0 )
      nl_error(3, "Out of memory in check" );
    for ( i = 0; i < colsz; i++ ) {
      newmdata[i] = &newvdata[i*rowsz];
    }
    if ( nrows != 0 && ncols != 0 ) {
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
