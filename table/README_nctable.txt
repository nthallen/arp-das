ntable.specs defines a processor to generate output that will run under ncurses.
The output file will look something like:

%{
/* nctable output */
#include "nctable.h"
static int idxdiag_winnum;

void idxdiag_redraw(void) {
  nct_clear(idxdiag_winnum);
  nct_string(idxdiag_winnum,r,c,a,str);
  nct_string(idxdiag_winnum,r,c,a,str);
  nct_string(idxdiag_winnum,r,c,a,str);
  nct_string(idxdiag_winnum,r,c,a,str);
}

void idxdiag_init(void) {
  idxdiag_winnum = nct_init("idxdiag", w, h);
  idxdiag_redraw();
}
%}
TM INITFUNC idxdiag_init();
nct_string(idxdiag_winnum,r,c,a,text(var));
