/* Definitions for TMCALGO V2.0 */
/* Do I need to include tmctime.h? */
#include <time.h>
#include "tmctime.h"
#ifndef _TMA_H_INCLUDED
#define _TMA_H_INCLUDED

typedef struct {
  long int dt;
  const char *cmd;
} tma_state;

typedef struct {
  int partno;
  const char *filename;
  const char *statename;
  tma_state *cmds;
  tma_state *def_cmds;
  time_t modtime;
} tma_ifile;

typedef struct {
  const char *state;
  const char *cmdstr;
} slurp_val;

extern slurp_val slurp_vals[];

/* tma_prtn.nexttime has changed def. between R1 and R2
   In R1: Seconds until next event, LONG_MAX if no next.
   In R2: Seconds from basetime of next event, 0 if no next.
*/
typedef struct {
  long int basetime; /* Time current state began */
  long int nexttime; /* see above */
  long int lastcheck; /* Time of last check */
  int console;
  int row;
  /* R2 additions: */
  tma_state *cmds;
  int next_cmd;
  long int waiting;
  const char *next_str;
} tma_prtn;
extern tma_prtn *tma_partitions;
extern long int tma_runbasetime;
extern int tma_is_holding;
extern const int tma_n_partitions;

#define HOLDING_COL 18
#define STATE_COL 42
#define RUN_COL 59
#define SNAME_COL 0
#define NEXTTIME_COL 34
#define STATETIME_COL 51
#define RUNTIME_COL 66
#define RUNTIME_STRING "             "
#define HOLDING_ATTR 0x70
#define TMA_ATTR 0x70
#define CMD_ATTR 7

#ifdef __cplusplus
extern "C" {
#endif

/* tmcalgo (tma) support routines */
void tma_new_state(unsigned int partition, const char *name);
void tma_init_options(int argc, char **argv);
void tma_hold(int hold);
void tma_time(tma_prtn *p, unsigned int col, long int t);
void tma_next_cmd( unsigned int partition, const char *cmd );

/* R2 routines */
void tma_init_state( int partno, tma_state *cmds, const char *name );
void tma_read_file( tma_ifile *ifilespec );
int tma_process( long int it );
void tma_succeed( int partno, int statecase );

#ifdef __cplusplus
};
#endif

#endif /* _TMA_H_INCLUDED */
