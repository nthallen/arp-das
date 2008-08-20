/* tma.c Defines TMA support services */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "nortlib.h"
#include "tmctime.h"
#if HAVE_NL_CONS_H
  #include "nl_cons.h"
#endif
#include "tma.h"
#include "oui.h"
char rcsid_tma_c[] = 
  "$Header$";

tma_prtn *tma_partitions = NULL;
long int tma_runbasetime = 0L;
int tma_is_holding = 0;

// To be superceded by tmc-generated function
long itime(void) { return 0L; }
slurp_val slurp_vals[1] = { NULL, 0 };
int tma_n_partitions = 0;

/*
>Command Text Goes Here
Initialize        Holding   Next: 00:00:00  State: 00:00:00  Run: 00:00:00
SNAME             HOLDING   NEXT  NEXTTIME  STATE  STATETIME RUN  RUNTIME
*/

void tma_time(tma_prtn *p, unsigned int col, long int t) {
  char ts[9];
  int hh, h;
  
  if (p->row >= 0) {
    #if HAVE_NL_CONS_H
	ts[8] = '\0';
	hh = t % 60;
	h = hh % 10;
	ts[7] = h+'0';
	ts[6] = hh/10 + '0';
	ts[5] = ':';
	t /= 60;
	hh = t % 60;
	h = hh % 10;
	ts[4] = h+'0';
	ts[3] = hh/10 + '0';
	ts[2] = ':';
	t /= 60;
	hh = t % 100;
	h = hh % 10;
	ts[1] = h+'0';
	ts[0] = hh/10 + '0';
	nlcon_display(p->console, p->row + 1, col, ts, TMA_ATTR);
    #endif
  }
}

static void update_holding(tma_prtn *p) {
  #if HAVE_NL_CONS_H
  if (p->row >= 0)
	nlcon_display(p->console, p->row + 1, HOLDING_COL,
	  tma_is_holding ? "Holding" : "       ", HOLDING_ATTR);
  #endif
}

void tma_hold(int hold) {
  int i, update;

  update = 0;
  if (hold) {
	if (! tma_is_holding) {
	  tma_is_holding = 1;
	  update = 1;
	}
  } else {
	if (tma_is_holding) {
	  tma_is_holding = 0;
	  update = 1;
	}
  }
  #if HAVE_NL_CONS_H
  if (update)
	for (i = 0; i < tma_n_partitions; i++) update_holding(&tma_partitions[i]);
  #endif
}

void tma_new_state(unsigned int partition, const char *name) {
  tma_prtn *p;

  if (partition < tma_n_partitions) {
	int log_level = -2;
	if ( *name == '_' ) {
	  name++;
	  log_level--;
	}
	nl_error( log_level, "%sEntering State %s", ci_time_str(), name );
	p = &tma_partitions[partition];
	p->basetime = (tma_runbasetime == 0L) ? 0L : itime();
	p->lastcheck = p->basetime;
	p->nexttime = 0;
	#if HAVE_NL_CONS_H
	if (p->row >= 0) {
	  nlcon_display(p->console, p->row + 1, STATE_COL,
		"  State: ", TMA_ATTR);
	  nlcon_display(p->console, p->row + 1, RUN_COL,
		"  Run: ", TMA_ATTR);
	  nlcon_display(p->console, p->row + 1, SNAME_COL,
		"                            Next:         ", TMA_ATTR);
	  nlcon_display(p->console, p->row + 1, SNAME_COL,
		name, TMA_ATTR);
	  nlcon_display(p->console, p->row + 1, RUNTIME_COL,
		RUNTIME_STRING, TMA_ATTR);
	  update_holding(p);
	  tma_time(p, STATETIME_COL, 0);
	}
	#endif
  }
}

/* specify the next significant time for a partition */
void tma_next_cmd(unsigned int partn, const char *next_cmd) {
  #if HAVE_NL_CONS_H
  tma_prtn *p;
  const char *txt;
  int len;
  
  if (partn < tma_n_partitions) {
	p = &tma_partitions[partn];
	if (p->row >= 0) {
	  nlcon_display(p->console, p->row, 0, ">", TMA_ATTR);
	  nlcon_display(p->console, p->row, 1,
		"                                        "
		"                                       ", CMD_ATTR);
	  len = strlen(next_cmd);
	  if (len > 79) txt = next_cmd + (len - 79);
	  else txt = next_cmd;
	  nlcon_display(p->console, p->row, 1, txt, CMD_ATTR);
	}
  }
  #endif
}

void tma_init_options(int argc, char **argv) {
  int c, con_index, part_index;

  tma_partitions = malloc(sizeof(tma_prtn) * tma_n_partitions);
  if (tma_partitions == NULL)
	nl_error(3, "No memory for partitions table");
  for (c = 0; c < tma_n_partitions; c++) {
	tma_partitions[c].row = -1;
	tma_partitions[c].cmds = NULL;
	tma_partitions[c].next_cmd = -1;
	tma_partitions[c].waiting = 0;
	tma_partitions[c].next_str = NULL;
  }

  optind = OPTIND_RESET; /* start from the beginning */
  opterr = 0; /* disable default error message */
  con_index = 0;
  part_index = 0;
  while ((c = getopt(argc, argv, opt_string)) != -1) {
	switch (c) {
	  #if HAVE_NL_CONS_H
	  case 'a': /* move on to the next console */
		con_index++;
		break;
	  case 'r':
		if (part_index < tma_n_partitions) {
		  tma_partitions[part_index].row = atoi(optarg);
		  tma_partitions[part_index].console = con_index;
		  part_index++;
		}
		break;
	  #endif
	  case '?':
		nl_error(3, "Unrecognized Option -%c", optopt);
	}
  }
}
