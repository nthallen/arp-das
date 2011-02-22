/* read_val.c reads valving or set point characters.
 * $Log$
 * Revision 1.1  2011/02/21 18:26:05  ntallen
 * QNX4 version
 *
 * Revision 1.3  1993/09/28 17:14:59  nort
 * *** empty log message ***
 *
 * Revision 1.2  1993/09/28  17:07:14  nort
 * *** empty log message ***
 *
   Written March 24, 1987
   Modified July 1991 for QNX.
*/
#include <stdio.h>
#include <ctype.h>
#include "solenoid.h"
#include "dtoa.h"
#include "proxies.h"
#include "tokens.h"

static char rcsid[] =
      "$Id$";

int open_char = 'O';
int close_char = '_';
int switch_char = '^';

int get_change_code(int type, int dtoa_num) {
  int c, i;

  for (;;) {
    c = gt_getc();
    if (c == '\n' || c == EOF) return(-1);
    if (c == switch_char) return(MODE_SWITCH_OK);
    if (type == TK_SOLENOID_NAME) {
      if (c == open_char) return(SOL_OPEN);
      if (c == close_char) return(SOL_CLOSE);
    } else if (!isspace(c)) {
	  if (type == TK_DTOA_NAME) {
		for (i = dtoas[dtoa_num].n_set_points - 1; i >= 0; i--)
		  if (c == dtoas[dtoa_num].set_point_name[i])
			return(i);
	  } else if (type == TK_PROXY_NAME)
		for (i = proxies[dtoa_num].n_proxies - 1; i >= 0; i--)
		  if (c == proxies[dtoa_num].proxy_name[i])
			return(i);
	}
  }
}
