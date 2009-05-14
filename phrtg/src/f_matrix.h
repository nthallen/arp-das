#ifndef F_MATRIX_H_INCLUDED
#define F_MATRIX_H_INCLUDED
#include <stdio.h>

typedef float scalar_t;
typedef scalar_t *vector_t;
typedef vector_t *matrix_t;

class f_matrix {
  public:
    vector_t vdata;
    matrix_t mdata;
    unsigned nrows;
    unsigned ncols;
    int offset;

    inline f_matrix(unsigned rowsize, unsigned colsize) { init(rowsize, colsize); }
    inline f_matrix(unsigned rowsize) { init( rowsize, 1 ); }
    inline f_matrix() { init( 0, 0 ); }
    f_matrix( char *filename, int format );
    void init( unsigned rowsize, unsigned colsize );
    void read_text( char *filename, unsigned minrows );
    void read_icos( FILE *fp );
    void read_icos( const char *filename );
    void append( float value );
    void check( unsigned rowsize, unsigned colsize, bool preserve = true );
    void check( unsigned vecsize, bool preserve = true );
    void setsize( unsigned nrows, unsigned ncols, bool preserve = true );
    inline void clear() { setsize( 0, 0 ); }
    int length();

  private:
    unsigned maxrows;
    unsigned maxcols;
};
#define FM_FMT_TEXT 1
#define FM_FMT_ICOS 2
#define MYBUFSIZE 1024

#endif
