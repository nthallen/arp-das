Things to accomplish for TMCALGO v2.0
Syntax to make commands, states silent {
  State names ending with '_' and commands
  beginning with '_' will not be logged.
  The state name kluge is sub-optimal since the change
  from logging to silent requires every reference to the state
  name to be changed. It would be much more helpful if there
  were a syntax at the declaration that would mark the state as
  being silent.
}
Bugs/Features {
  Messages need to be cleared more consistently {
	Could hold onto the currently displayed message. Then we 
	would know if a new one needs to be displayed.
  }
  Commands should not be sorted within time intervals {
	This is confusing and I don't think it serves the original 
	purpose.
  }
}
New Feature Wish List {
  hold unless valid (); (pertains to whole partition)
  Syntax to make commands, states silent
  Figure out how to make internal commands execute exactly once
  states.c {
	Perhaps commands following a hold until should be deemed to
	belong to another time period. Definitely. Probably already 
	true.
  }
  Library {
	During playback, report itime() in all messages?
  }
  { Would it be worthwhile to support an option to execute an 
	internal TMC statement more than once? I think not.
  }
}
Documentation Notes {
  Untimed commands consist entirely of tmc_statements
  and depending on statements. ">" and "qstring" commands are
  all timed commands, even at T=0, so if you would like a
  tmc_statement to be a timed command at T=0, preceed it with
  a qstring or ">" command.

  TMC statements within {} lose their curlies when going into the 
  TMC file. Double braces are required if a compound TMC 
  statement is desired.
  
  Within one time period, commands are sorted by type
    Regular commands
	Explicit Validations
	Qstrings
    TMC commands (i.e. validation of appropriate substate )
	Conditionals
  You may wish to sort your commands this way also to avoid 
  confusion.
  
  Note that TMC statements at the same time as conditionals
  (hold until) in all likelihood won't be executed, particularly 
  if they have time constraints since their substates are 
  validated and then immediately invalidated. By the same token, 
  TMC statements in the else-clause of a conditional will have a 
  similarly limited execution window. As such, they should be 
  limited to C statements with no TM dependencies ( msg() for 
  example).
}
New syntax elements {
  +depending on ( <validation list> ) { <statement> ... }
  +"quoted string";
  +state <name> "filename" [ ( <statename> ... ) ] ;
  # hold until valid ( <validation list> ) [ or <n> [ else <cmd> ]];
  # hold until ( <expr> );
  Validate TK_NAME;
  
  +TMC statements bracketed by {} instead of []
  +Quoted strings displayed instead of next command
  +%{, %} support to pass straight through to C
}
Hold until syntax {
	KW_HOLD KW_UNTIL hold_cond hold_term
  hold_cond : ';'
	  | KW_OR TK_NUMBER else_clause
	  ;
  else_clause : ';'
	  | KW_ELSE timed_command
	  ;
  Implementation {
    Compiled code must record timeout period and what commands
	to execute on success and failure
	Clearly when a hold is encountered:
	  The hold must be recorded in the partition
	  The holding substate must be validated
	  tma_process must monitor for terminal count.
	  If the hold until condition is met, the holding substate 
	  must be invalidated, probably by validating the partition's 
	  idle substate. Also a hold_end() routine should be called
	  to update the next command table, presumably advancing to 
	  the success command.
	  If terminal count is reached, table is modified to advance 
	  to the fail command. Since there are no commands in the 
	  condition portion of hold until, nesting does not get 
	  terribly complicated.
	  
  }
}
Implementation {
  States are table-driven. Special substates are only required 
  for TMC statements
  
  Each partition has only one active state, so all the elapsed 
  time, etc. can be kept with the partition

  State maintains a command table recording
	Elapsed time for command: must be long
	Negative time means end of state
	String
	  Actual command strings
	  Quoted Strings
	  Substate values
	First char of command string could record the command type
	 #"T"  State's Title (Initialize, etc.) (passed as arg instead)
	 "\"" Quoted string for display
	 ">"  CMDGEN command
	 "#"  New Substate case Value
	      New substate could be stored as ascii "#35"
	 NULL End of State

	New substate info is returned. When no more data can be 
	processed at the present time, zero is returned.
	Partition struct should record the current substate
	
	Compile Time:

	partitions[] {
	  states[] {
	    name
		commands[] {
		  long int ctime;
		  char *cmd;
		}
	  }
	  substates[]
	}
	
	Run Time:

	partitions[] {
	  states[]
	}

    %{
	  /* version stuff */
	%}
    %{
	  #include "tmctime.h"
	  const int tma_n_partitions = 2;
	%}
	
	state ( foo, bar );
	validate foo;
	state ( foo_0, foo_1, foo_2, bar_0, bar_1 );

    tma_state st_foo_cmds[] = {
	  0, "\"A Message Goes Here",
	  0, ">Telemetry Logging Suspend",
	  0, "#1",
	};

	Output QStrings first, then commands, then validations.

    depending on ( st_foo once ) {
	  initialize_state( partition_number, st_foo_cmds);
    }

/* initialize_state() will send (optional) message
   about entering the new state and will modify the
   parition header information and status display,
   but will not execute any commands.

   tma_init_options() will need to call tma_process() to execute 
   startup commands (Telemetry Start, etc.) Cannot validate 
   substates during initialization. May have to rely on the fact 
   that T=0 substates can be defined statically.
   
   Alternate approach is to have initial substates handled as
   at present: depending on ( st_foo once ) {
	 init_state( st_foo ); validate substate0; }
   This is somewhat more confusing, since substate validations 
   take place in multiple places.

	tma_process() will have to deal with the fact that I don't know 
	the starting time when called from the initialization. Then 
	again, initialize_state() shouldn't need to report the time 
	either, even though you might think that is a logical place to 
	define the starting time. initialize_state should set the 
	partition's state base time to 0, say, to indicate that it isn't 
	set. When tma_process is called with a non-zero time, the zeros 
	are automatically promoted. 1 would have to be recognized also as 
	meaning "I've already processed the T=0 commands, but I still 
	don't know the time." The basetime would then be set to the 
	current time - 1.

    First state in partition is default and will be validated 
	statically. First substate within that state is default and 
	will be validated statically. (currently substates are not 
	statically validated since they are validated within the 
	state's init).
*/
    depending on ( st_foo ) {
	  <top statement>
	}
	
	depending on ( <substate> once ) {
	  <middle statement>
	}
	
	depending on ( <last state substate> ) {
	  <end statement>
	}

	depending on ( 1 Hz ) {
	  long int it;
	  int subcase;
	  
	  it = itime();
	  while ( subcase = tma_process( it ) ) {
		switch (subcase) {
		  case 1: validate substate_1; break;
		  ...
		  default: nl_error( 1, "Unexpected return value from 
		  tma_process" );
		}
	  }
	}

    tma_process() will hide all looping through partitions.
	It will continue looping until no partitions require any
	further commands. This may require revisiting some 
	partitions, since partition 2 may validate a substate
	which validates a state in partition 1 which will
	immediately reinitialize the partition structure, but will 
	not execute any commands, though it should be possible to get 
	them executed in this loop.
}
Library {
  Modify tma.c. Make local functions static, eliminating 
  tma_sendcmd() at the least. tma_init_options() remains the 
  same. Partition structure needs some additional members. Add 
  tma_process() function, etc.
  
  Partition tma.c into functions which are common to 
  new and old implementation, then create a new file also. That 
  way the new library should be compatible with old and new 
  versions.

  New functions etc.
	typedef struct {
	  long int time;
	  char *cmd;
	} tma_state;
    tma_init_state( int partno, tma_state *cmds, char *statename );
	tma_slurp( int partno, char *filename, char *statename );
	  opens file, reads in command structure, checks the 
	  command's syntax, then calls tma_init_state();
	tma_process( long itime );
}
{ From tma.c I think we can just extract tma_time_check() and
  tma_sendcmd() and use all the rest.
}
{ void tma_init_state( int partno, tma_state *cmds, char *name );
	initialize partition array if necessary
	struct {
	  tma_state *cmds;
	  int next_cmd;
	  int waiting;
	}
}
