/* read_mode.c reads in the mode commands.
 * $Log$
 * Revision 1.4  2006/02/16 18:13:26  nort
 * Uncommitted changes
 *
 * Revision 1.3  1993/09/28  17:14:48  nort
 * *** empty log message ***
 *
 * Revision 1.2  1993/09/28  17:07:08  nort
 * *** empty log message ***
 *
   Written March 24, 1987
   Modified July 1991 for QNX.
   Modified 4/17/92 for QNX 4.
*/
#include <stdio.h>
#include <malloc.h>
#include "tokens.h"
#include "modes.h"
#include "solenoid.h"
#include "proxies.h"
#include "routines.h"
#include "dtoa.h"
#include "solfmt.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

/* I want this to be generalized to sort changes by time and type
   The types are '^', TK_PROXY_NAME, TK_DTOA_NAME, TK_SOLENOID_NAME
   New change preceeds another if the new time is less than the other
   or new type is greater than the other.
 */
void add_change(change *nc, change **base) {
  change *oc, *noc;

  oc = NULL;
  noc = *base;
  while (noc != NULL
		 && (nc->time > noc->time
			 || (nc->time == noc->time
				 && nc->type <= noc->type))) {
	oc = noc;
	noc = noc->next;
  }
  nc->next = noc;
  if (oc == NULL) *base = nc;
  else {
	if (oc->time == nc->time
		&& oc->type == nc->type
		&& oc->t_index == nc->t_index
		&& oc->state == nc->state)
	  free(nc);
	else oc->next = nc;
  }
}

mode modes[MAX_MODES];
int n_solenoids = 0;
int n_dtoas = 0;
solenoid solenoids[MAX_SOLENOIDS];
dtoa dtoas[MAX_DTOAS];
int res_num = 1, res_den = 1;

void init_modes(void) {
  int i;

  for (i = 0; i < MAX_MODES; i++) {
    modes[i].init = modes[i].first = NULL;
    modes[i].next_mode = -1;
  }
}

static change *new_change(int type, int state) {
  change *ch;
  
  ch = malloc(sizeof(change));
  if (ch == NULL) filerr("No memory");
  ch->type = type;
  ch->state = state;
  return(ch);
}

void read_mode(void) {
  int mn, i, sn, time, token, in_routine, flin;
  long int fpos;
  change *nc;

  in_routine = 0;
  if (get_token() != TK_NUMBER) filerr("Mode requires number\n");
  mn = gt_number;
  if (mn < 0 || mn > MAX_MODES) filerr("Mode number out of range\n");
  if (modes[mn].init != NULL || modes[mn].first != NULL ||
        modes[mn].next_mode >= 0)
    filerr("Attempted Redefinition of mode %d\n", mn);
  if (get_token() != TK_LBRACE) filerr("Mode requires Left Brace\n");
  for (i = 0; i < n_solenoids; i++) solenoids[i].last_time =
    solenoids[i].last_state = solenoids[i].first_state = -1;
  for (i = 0; i < n_dtoas; i++) dtoas[i].last_time =
	dtoas[i].last_state = dtoas[i].first_state = -1;
  for (i = 0; i < n_proxies; i++) proxies[i].last_time =
	proxies[i].last_state = proxies[i].first_state = -1;
  for (;;) {
    token = get_token();
    switch (token) {
      case TK_INITIALIZE:
        token = get_token();
        if (token != TK_SOLENOID_NAME
			&& token != TK_DTOA_NAME
			&& token != TK_PROXY_NAME)
          filerr("Initialize requires solenoid, DtoA or Proxy name\n");
        sn = gt_number;
        if (get_token() != TK_COLON) filerr("Initialize <name>: - ':' needed\n");
        if ((i = get_change_code(token, sn)) < 0)
          filerr("Initialize <name>:<valving char> - No <valving char>\n");
        nc = new_change(token, i);
		nc->time = -1;
        nc->t_index = sn;
		add_change(nc, &modes[mn].init);
        continue;
      case TK_SOLENOID_NAME:
      case TK_DTOA_NAME:
	  case TK_PROXY_NAME:
        sn = gt_number;
        if (get_token() != TK_COLON) filerr("<name>: - Missing the ':'\n");
        if (token == TK_SOLENOID_NAME) time = solenoids[sn].last_time;
        else if (token == TK_DTOA_NAME) time = dtoas[sn].last_time;
        else if (token == TK_PROXY_NAME) time = proxies[sn].last_time;
        for (;;) {
		  i = get_change_code(token, sn);
          if (i == -1) break;
          if (time == -1) time = 0;
          else if (i != MODE_SWITCH_OK) time++;
          if (i != MODE_SWITCH_OK
              && ((token == TK_DTOA_NAME
				   && i == dtoas[sn].last_state)
				  || (token == TK_SOLENOID_NAME
					  && i == solenoids[sn].last_state)
				  || (token == TK_PROXY_NAME
					  && i == proxies[sn].last_state)))
            continue;
		  if (i == MODE_SWITCH_OK) {
			nc = new_change('^', 0);
            nc->t_index = 0;
            nc->time = time+1;
          } else {
			nc = new_change(token, i);
            nc->t_index = sn;
            nc->time = time;
			switch (token) {
			  case TK_SOLENOID_NAME:
				if (solenoids[sn].first_state == -1)
				  solenoids[sn].first_state = i;
				solenoids[sn].last_state = i;
				break;
			  case TK_DTOA_NAME:
				if (dtoas[sn].first_state == -1) dtoas[sn].first_state = i;
				dtoas[sn].last_state = i;
				break;
			  case TK_PROXY_NAME:
				if (proxies[sn].first_state == -1)
				  proxies[sn].first_state = i;
				proxies[sn].last_state = i;
				break;
            }
		  }
          add_change(nc, &modes[mn].first);
        }
		switch (token) {
		  case TK_SOLENOID_NAME:
			solenoids[sn].last_time = time;
			break;
		  case TK_DTOA_NAME:
			dtoas[sn].last_time = time;
			break;
		  case TK_PROXY_NAME:
			proxies[sn].last_time = time;
			break;
		}
        continue;
      case TK_ROUTINE_NAME:
        if (in_routine) filerr("Nested routines are not supported\n");
        fpos = gt_fpos(&flin);
        gt_spos(routines[gt_number].fpos, routines[gt_number].flin);
        in_routine = 1;
        continue;
      case TK_SELECT:
        if (in_routine) filerr("Select within routine is illegal\n");
        if (get_token() != TK_NUMBER) filerr("Select expects mode number\n");
        modes[mn].next_mode = gt_number;
        if (get_token() != TK_RBRACE)
          filerr("Select must be last command in mode\n");
        break;
      case TK_RBRACE:
        if (in_routine) {
          gt_spos(fpos, flin);
          in_routine = 0;
          continue;
        } else break;
      default: filerr("Unexpected token in read_mode\n");
    }
    break; /* only for RBRACE */
  }
  time = -1;
  for (i = 0; i < n_solenoids; i++)
    if (solenoids[i].last_time != -1)
      if (time == -1) time = solenoids[i].last_time;
      else if (solenoids[i].last_time != time)
        filerr("Mode %d has ambiguous cycle lengths (sol. %s)\n", mn,
                solenoids[i].name);
  for (i = 0; i < n_dtoas; i++)
    if (dtoas[i].last_time != -1)
      if (time == -1) time = dtoas[i].last_time;
      else if (dtoas[i].last_time != time)
        filerr("Mode %d has ambiguous cycle lengths (dtoa %s)\n", mn,
                dtoas[i].name);
  for (i = 0; i < n_proxies; i++)
    if (proxies[i].last_time != -1)
      if (time == -1) time = proxies[i].last_time;
      else if (proxies[i].last_time != time)
        filerr("Mode %d has ambiguous cycle lengths (proxy %s)\n", mn,
                proxies[i].name);
  modes[mn].length = time+1;    /* length of the cycle */
  modes[mn].res_num = res_num;
  modes[mn].res_den = res_den;
  optimize(mn);
}
