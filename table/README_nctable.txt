nctable.specs defines a processor to generate output that will run
under ncurses.  The output file will look something like:

%{
/* nctable output */
#include "nctable.h"
static int idxdiag_winnum;
static unsigned char nct_r1[] = { 2, 2, 2, 3, 0 };
static unsigned char nct_r2[] = { 1, 1, 1, 1, 0 };

void idxdiag_redraw(void) {
  nct_clear(idxdiag_winnum);
  nct_string(idxdiag_winnum,a,r,c,str);
  nct_string(idxdiag_winnum,a,r,c,str);
  nct_string(idxdiag_winnum,a,r,c,str);
  nct_string(idxdiag_winnum,a,r,c,str);
  nct_hrule( idxdiag_winnum, a, r, c, nct_r1 );
  nct_vrule( idxdiag_winnum, a, r, c, nct_r2 );
}

void idxdiag_init(void) {
  idxdiag_winnum = nct_init("idxdiag", w, h);
  idxdiag_redraw();
}
%}
TM INITFUNC idxdiag_init();
nct_string(idxdiag_winnum,r,c,a,text(var));
