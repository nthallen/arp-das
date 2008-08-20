#include <stdlib.h>
#include "nortlib.h"
#include "tma.h"
char rcsid_tmaR2_c[] = 
  "$Header$";

void tma_init_state( int partno, tma_state *cmds, char *name ) {
  tma_prtn *p;

  if ( partno < tma_n_partitions ) {
	/* tma_new_state() sets part basetime, outputs STATETIME
	   Does *not* set RUNTIME or NEXTTIME or next command area
	   basetime is set only if tma_basetime is non-zero (set
	   in tma_process first time after TM starts). lastcheck is 
	   set to basetime. partno, name );
	 */
	tma_new_state( partno, name );
	p = &tma_partitions[partno];
	p->cmds = cmds;
	p->next_cmd = 0;
	p->waiting = 0;
	p->lastcheck = p->basetime - 1;
	if ( p->basetime == 0L && p->cmds != 0 ) {
	  /* Issue start-up commands (T==0, ">") */
	  for (;;) {
		cmds = &p->cmds[p->next_cmd];
		if ( cmds->dt != 0 || cmds->cmd[0] != '>' )
		  break;
		p->next_cmd++;
		if ( cmds->cmd[1] == '_' ) ci_sendcmd(cmds->cmd+2, 2);
		else ci_sendcmd(cmds->cmd+1, 0);
	  }
	}
  }
}

/* tma_process returns 0 when no more action is required for any
   partition at the present time (now). Otherwise it returns
   a positive integer which is an index into case statement
   generated by tmcalgo for validating states. */
int tma_process( long int now ) {
  int partno, success, state_case, Rpartno;
  tma_prtn *p;
  long int dt, pdt, timeout, lastcheck;
  char *cmd;
  
  if (tma_runbasetime == 0L) {
	unsigned int i;

	tma_runbasetime = now;
	for (i = 0; i < tma_n_partitions; i++) {
	  tma_partitions[i].basetime = now;
	  tma_partitions[i].lastcheck = now-1;
	}
  }
  for ( partno = 0; partno < tma_n_partitions; partno++ ) {
	p = &tma_partitions[ partno ];
	if ( now != p->lastcheck ) {
	  lastcheck = p->lastcheck;
	  if ( lastcheck < p->basetime )
		lastcheck = p->basetime;
	  if ( p->waiting > 0 ) {
		dt = now - lastcheck;
		if ( dt >= p->waiting ) {
		  p->waiting = 0;
		  p->next_cmd++;
		}
	  }
	  if ( p->waiting == 0 && tma_is_holding == 0 ) {
		dt = now - p->basetime;
		if ( p->cmds != 0 && p->next_cmd >= 0 ) {
		  while ( p->waiting == 0 ) {
			pdt = p->cmds[p->next_cmd].dt;
			cmd = p->cmds[p->next_cmd].cmd;
			if ( pdt == -1 ) {
			  p->next_cmd = -1;
			  p->nexttime = 0;
			  break;
			} else if ( pdt > dt ) {
			  if ( p->nexttime != pdt ) {
				p->nexttime = pdt;
				if ( ( p->next_str == 0 || *p->next_str == '\0' )
					  && *cmd == '>' )
				  p->next_str = cmd+1;
			  }
			  break;
			} else {
			  p->next_cmd++;
			  switch( *cmd ) {
				case '>':
				  if ( cmd[1] == '_' )
					ci_sendcmd(cmd+2, 2);
				  else
					ci_sendcmd(cmd+1, 0);
				  p->next_str = "";
				  break;
				case '"':
				  p->next_str = cmd;
				  break;
				case '#':
				  return atoi( cmd+1 );
				case '?':
				  if ( sscanf( cmd+1, "%d,%ld,%d",
					&success, &timeout, &state_case ) != 3 )
					nl_error( 4, "Error reading hold command\n" );
				  if ( timeout > 0 )
					timeout += pdt + p->basetime - lastcheck;
				  p->waiting = timeout;
				  p->next_cmd--;
				  if ( state_case > 0 ) return state_case;
				  break;
				case 'R':
				  if ( sscanf( cmd+1, "%d,%d", &Rpartno,
						&state_case ) != 2 )
					nl_error( 4, "Error reading Resume command\n" );
				  tma_succeed( Rpartno, -state_case );
				  if ( Rpartno < partno ) return -1;
				  break;
				default:
				  nl_error( 1, "Unknown cmd char %c in tma_process", *cmd );
			  }
			}
		  }
		}
	  }
	}
  }
  for ( partno = 0; partno < tma_n_partitions; partno++ ) {
	p = &tma_partitions[ partno ];
	lastcheck = p->lastcheck;
	if ( lastcheck < p->basetime )
	  lastcheck = p->basetime;
	dt = now - lastcheck;
	if ( dt != 0 ) {
	  if ( p->waiting != 0 || tma_is_holding != 0 ) {
		p->basetime += dt;
		if ( p->waiting > dt ) {
		  p->waiting -= dt;
		} else if ( p->waiting > 0 )
		  p->waiting = 0;
	  }
	}
	if ( now != p->lastcheck ) {
	  p->lastcheck = now;
	  if ( p->row >= 0 ) {
		if ( p->next_str != 0 ) {
		  tma_next_cmd( partno, p->next_str );
		  p->next_str = NULL;
		}
		tma_time( p, RUNTIME_COL, now - tma_runbasetime );
		tma_time( p, STATETIME_COL, now - p->basetime );
		pdt = p->nexttime;
		switch ( pdt ) {
		  case -1: continue;
		  case 0:
			pdt = 0;
			p->nexttime = -1;
			break;
		  default:
			pdt += p->basetime - now;
			break;
		}
		tma_time( p, NEXTTIME_COL, pdt );
	  }
	}
  }
  return 0; /* Nothing left to do */
}

void tma_succeed( int partno, int statecase ) {
  tma_prtn *p;
  int success, state_case, res_case;
  long int timeout;
  char *cmd;

  if ( partno < 0 || partno >= tma_n_partitions )
	nl_error( 4, "Invalid partno in tma_succeed" );
  p = &tma_partitions[ partno ];
  if ( p->cmds != 0 && p->next_cmd >= 0 ) {
	cmd = p->cmds[p->next_cmd].cmd;
	if ( cmd != 0 && *cmd == '?' ) {
	  int n_args = sscanf( cmd+1, "%d,%ld,%d,%d",
		  &success, &timeout, &state_case, &res_case );
	  if ( n_args < 3 ) nl_error( 4, "Error re-reading hold command\n" );
	  if ( statecase > 0 && state_case != statecase )
		nl_error( 1, "Different statecase active in tma_succeed" );
	  else if ( statecase < 0 &&
				( n_args < 4 || -statecase != res_case) ) {
		if ( n_args < 4 )
		  nl_error( 1, "Resume not compatible with this version" );
		else
		  nl_error( -3, "Resume(%d,%d): other state is holding", partno,
					  -statecase );
	  } else {
		p->waiting = 0;
		p->next_cmd += success;
	  }
	  return;
	}
  }
  if ( statecase < 0 )
	nl_error( -3, "Resume(%d,%d): Specified state not holding",
		partno, -statecase );
  else nl_error( -3, "Hold(%d,%d) not active in tma_succeed",
		partno, statecase );
}

/*
=Name  <funcname>: <Description>
=Name tma_init_state(): Initialize a new TMA state
=Subject TMA Internal
=Name tma_process(): Execute TMA commands
=Subject TMA Internal
=Name tma_succeed(): End TMA Hold states
=Subject TMA Internal
=Synopsis
#include "tma.h"
void tma_init_state( int partno, tma_state *cmds, char *name );
int tma_process( long int now );
void tma_succeed( int partno, int statecase );

=Description

These are support functions for TMCALGO algorithms. They are
called from the .tmc code which TMCALGO generates. Users should
not need to call these functions directly.

tma_init_state() is called to when a TMA state is validated.
It is responsible for initializing the status lines and setting
up the internal structures which keep track of the current
position within the algorithm.

tma_process() is called every second to execute algorithm
commands. Its single argument is the "current" data stream
time. (This will be very close to the current time when
running an algorithm in realtime, but during playback, it will
be the time when the data was taken.)

tma_succeed() is called to release a partition from a HOLD
state. If statecase is greater than zero, it must match
the third argument of the currently pending hold command.
This form is generated by "Hold until" syntaxes. If statecase
is less than zero, it must equal (but negative) to the fourth
argument. This form is generated by the "Resume" statement.

=Returns

tma_process() returns zero if there is no more work to be done at
this time. If it returns a non-zero value, the TMCALGO-generated
.tmc program uses the value in a switch statement to select a
state to validate. (This value is referred to as a "state_case"
within the tmcalgo source code.) A return value of -1 indicates
that work is not completed, but no action is required by the
caller other than to call tma_process() again. This is necessary
when a "Resume" command is executed, which may unblock a
partition that has already been passed over.

tma_init_state() and tma_succeed() have void returns.

=SeeAlso

=TMA Internal= functions.

=End
*/