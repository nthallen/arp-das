#ifndef NCTABLE_H_INCLUDED
#define NCTABLE_H_INCLUDED

#define NCT_CHARS_GR 0
#define NCT_CHARS_ASCII 1

extern void nct_args( char *dev_name );
extern int nct_init( char *winname, int n_rows, int n_cols );
extern void nct_refresh(void);
extern void nct_string( int winnum, int attr, int row, int col, char *text );
extern void nct_clear( int winnum );
extern void nct_charset(int n);
extern void nct_hrule( int winnum, int attr, int row, int col,
		unsigned char *rule );
extern void nct_vrule( int winnum, int attr, int row, int col,
		unsigned char *rule );

#endif
