#ifndef PORT_H_INCLUDED

extern int stch_i( const char *str, int *rv );
extern int stcpm( const char *str, char *pat, char **match);
extern int stcpma(const char *str, const char *pat);
#define fopene(x,y,z) fopen(x,y)

#endif
