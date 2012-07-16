#ifndef NCTABLE_H_INCLUDED
#define NCTABLE_H_INCLUDED

#define NCT_CHARS_GR 0
#define NCT_CHARS_ASCII 1

#ifdef __cplusplus
  extern "C" {
#endif

extern void nct_init_options(int argc, char **argv);
extern void nct_args( char *dev_name );
extern void nct_charset(int n);
extern int nct_init( const char *winname, int n_rows, int n_cols );
extern int nct_cmdclt_init(void);
extern void nct_refresh(void);
extern void nct_string( int winnum, int attr, int row, int col,
		const char *text );
extern void nct_clear( int winnum );
extern void nct_hrule( int winnum, int attr, int row, int col,
		unsigned char *rule );
extern void nct_vrule( int winnum, int attr, int row, int col,
		unsigned char *rule );
extern char nct_getch(void);

#ifdef __cplusplus
  }
#endif

#endif
