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
    int nrows;
    int ncols;
    int offset;

    f_matrix(int rowsize, int colsize);
    inline f_matrix(int rowsize) { f_matrix( rowsize, 1 ); }
    inline f_matrix() { f_matrix( 0, 0 ); }
    f_matrix( char *filename, int format );
    void read_text( char *filename, int minrows );
    void read_icos( FILE *fp );
    void read_icos( const char *filename );
    void append( float value );
    void check( int rowsize, int colsize );
    void check( int vecsize );
    void setsize( int nrows, int ncols );
    inline void clear() { setsize( 0, 0 ); }
    int length();

  private:
    int maxrows;
    int maxcols;
};
#define FM_FMT_TEXT 1
#define FM_FMT_ICOS 2
#define MYBUFSIZE 1024

#endif
