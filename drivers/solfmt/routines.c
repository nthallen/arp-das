/* routines.c handles reading (swallowing) of routines and other routine
   administration.
   Written March 18, 1988
   Modified July 1991 for QNX.
*/
#include <stdio.h>
#include <string.h>
#include "tokens.h"
#include "routines.h"

rout routines[MAX_ROUTINES];
int n_routines = 0;

void read_routine(void) {
  int tk, c;

  if (get_token() != TK_UNDEFINED_STRING) filerr("Routine expected name\n");
  if (n_routines == MAX_ROUTINES) filerr("Too many routines\n");
  routines[n_routines].name = strdup(gt_input);
  if (get_token() != TK_LBRACE) filerr("Routine expected {\n");
  routines[n_routines].fpos = gt_fpos(&routines[n_routines].flin);
  n_routines++;
  for (;;) {
    tk = get_token();
    switch (tk) {
	  case TK_INITIALIZE:
		c = get_token();
		if ( c != TK_DTOA_NAME &&
			 c != TK_SOLENOID_NAME &&
			 c != TK_PROXY_NAME )
		 filerr("Illegal token following Initialize\n");
      case TK_DTOA_NAME:
      case TK_SOLENOID_NAME:
	  case TK_PROXY_NAME:
        for (;;) {
          c = gt_getc();
          if (c == '\n' || c == EOF) break;
        }
        break;
      case TK_RBRACE:
        return;
      default:
        filerr("Illegal token inside routine\n");
    }
  }
}
