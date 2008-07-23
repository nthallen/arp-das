/* tmctime.h defines time functions used in TMC outputs */

#define NEED_TIME_FUNCS
long itime(void);
double dtime(void);
double etime(void);
char *timetext(long int t);

