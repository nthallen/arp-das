/* compile.c takes the information in solenoids and modes and compiles it
   into a numerical code.
   $Log$
   Revision 1.1  2011/02/21 18:26:05  ntallen
   QNX4 version

   Revision 1.4  2006/02/16 18:13:21  nort
   Uncommitted changes

 * Revision 1.3  1993/09/28  17:14:28  nort
 * *** empty log message ***
 *
 * Revision 1.2  1993/09/28  17:06:49  nort
 * *** empty log message ***
 *
   Written March 25, 1987
   Modified April 10, 1987 for proper optimization.
   Modified July 1991 for QNX.
   July 1991: dosn't compile multiple SOL_STROBES and SOL_DTOA commands.
*/
#include <stdio.h>
#include <assert.h>
#include "tokens.h"
#include "modes.h"
#include "solenoid.h"
#include "dtoa.h"
#include "proxies.h"
#include "codes.h"
#include "solfmt.h"
#include "nortlib.h"

static char rcsid[] =
      "$Id$";

#define MODE_CODE_SIZE 10000
char mode_code[MODE_CODE_SIZE];
int mci = 0;    /* index into mode_code */
int verbose = 0;

void new_mode_code(int x) {
  if (mci == MODE_CODE_SIZE) nl_error(3, "Mode Code Table exceeded\n");
  mode_code[mci++] = x;
}

static change *compile_one(change *ch) {
  int t0;
  
  if (ch == NULL) return(NULL);
  t0 = ch->time;
  while (ch != NULL && ch->time == t0) {
	switch (ch->type) {
	  case TK_SOLENOID_NAME:
		{
		  change *nc;
		  int j;
		
		  for (j = 1, nc = ch->next;
			   nc != NULL
			   && nc->type == TK_SOLENOID_NAME
			   && nc->time == ch->time;
			   nc = nc->next) j++;
		  if (j == 1) new_mode_code(SOL_STROBES);
		  else {
			new_mode_code(SOL_MULT_STROBES);
			new_mode_code(j);
		  }
		  while (j-- > 0) {
			if (ch->state == SOL_OPEN)
			  new_mode_code(solenoids[ch->t_index].open_cmd);
			else new_mode_code(solenoids[ch->t_index].close_cmd);
			ch = ch->next;
		  }
		  break;
		case TK_DTOA_NAME:
		  new_mode_code(SOL_DTOA);
		  new_mode_code(dtoas[ch->t_index].set_point_index[ch->state]);
		  ch = ch->next;
		  break;
		case TK_PROXY_NAME:
		  new_mode_code(SOL_PROXY);
		  new_mode_code(proxies[ch->t_index].proxy_index[ch->state]);
		  ch = ch->next;
		  break;
      }
	} /* switch (ch->type) */
  } /* while (ch != NULL && ch->time == t0) */
  return(ch);
}

void compile(void) {
  int i, time, cycle_index;
  change *ch;

  if (verbose) describe();
  for (i = 0; i < MAX_MODES; i++) {
    if (modes[i].init == NULL && modes[i].first == NULL &&
        modes[i].next_mode < 0) {
      modes[i].index = -1;
      continue;
    }
    modes[i].index = mci;
	compile_one(modes[i].init);
    ch = modes[i].first;
    if (ch != NULL) {
      new_mode_code(SOL_SET_TIME);
      new_mode_code(modes[i].count & 0xFF);
      new_mode_code((modes[i].count >> 8) & 0xFF);
      cycle_index = mci;
      time = 0;
      while (ch != NULL) {
        comp_waits((ch->time - time) * modes[i].iters);
        time = ch->time;
        if (ch->type == '^') {
          new_mode_code(SOL_MSWOK);
          ch = ch->next;
        }
		ch = compile_one(ch);
      }
      comp_waits((modes[i].length - time) * modes[i].iters);
      time = modes[i].length;
      if (modes[i].next_mode >= 0) {
        new_mode_code(SOL_SELECT);
        new_mode_code(modes[i].next_mode);
      } else {
        new_mode_code(SOL_GOTO);
        new_mode_code(cycle_index & 0xFF);
        new_mode_code((cycle_index >> 8) & 0xFF);
      }
    }
    new_mode_code(SOL_END_MODE);
  }
}

void comp_waits(int j) {
  for (; j > 255; j -= 255) {
    new_mode_code(SOL_WAITS);
    new_mode_code(255);
  }
  if (j == 1) new_mode_code(SOL_WAIT);
  else if (j > 1) {
    new_mode_code(SOL_WAITS);
    new_mode_code(j);
  }
}
