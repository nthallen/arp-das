# mkdoit2.awk Generates a doit script
#	statusscreen (obsolete, may be omitted)
#	playback
#	inetin
#	autostart
#	batchfile <batch file name>
#	memo [ <log file name> ]
#		specifies that "more -e <log file>" should run on a spare console
#	display <program> <screen file> [ <screen file> ... ]
#		specifies a display program to run
#	extraction <program> [options]
#		specifies an extraction to run, including rtg extractions
#	algorithm <program> [options]
#		specifies an algorithm to run
#	client <program> [options]
#		specifies a keyboard client to run
#	monoconfig <filename>
#	colorconfig <filename>
#		specifies screen configuration files to use
#	rtg <config file>
#		start up rtg if it isn't already running
#
#  options may only be specified once for a particular program.
#----------------------------------------------------------------
# New version begun 9/12/95
#  No longer using "row". Will instead place screen items
#  based on the .fld files. Fields with text beginning with
#  % will designate instructions to mkdoit:
#   %STATUS:<app>   Indicates a field to locate the status
#                   for the specified application
#   %STATUS         Indicates a general status field for any
#                   applications without a specific status field
#   %CLIENT         Indicates that a keyboard client should run
#                   on this screen.
#   %TMA:<app>[:#]  Defines a field for the specified TMCALGO
#                   application's partition status. The
#                   partition number is optional to support
#                   status for multiple partitions. The
#                   partitions are numbered starting with 1.
# /* fields: number, line, col, width, length, attribute code, string */
# #FIELD# 152  4 72 5 1 4 "%TMA:hoxalgo:3"
# /* form: lines, cols, pos_y, pos_x */
# #FORM# 25 80 0 0
#----------------------------------------------------------------
# app[program] = "$_genopts"
# TM[program] = "$_dcopts"
# CMD[program] = "$_cmdopts"
# DISP[program] = "-A ..."
# CltScreens = " -A ..."
# opts[program] = "-l"
# Statline[program] = "-o ..."
# Statline["general"] = "-o ..."
# tma_con[ program, number ] = "$_scr0"
# tma_row[ program, number ] = 5
# tma_nparts[ program ] = largest partition number referenced
#
#  Every application goes on the app[] list and then is
#  added to the appropriate other lists. TM[] is for TM clients
#  CMD is for command clients, DISP is for display programs.
#  The actual value in that list is a string containing the
#  arguments appropriate to that particular program.
#
#  displays[n_displays] stores the names of the display programs
#  exts[n_exts] stores the names of the extractions
#  algos[n_algos] stores the names of the algorithms
#    All of these are 1-based.
#
#  disp_screens[n_displays] = number of screens/consoles for this display prog
#  disp_con[n_displays,i] = console number for this prog's i'th screen
#  disp_fld[n_displays,i] = fld file for this prog's i'th screen
#    All indicies here are 1-based.
#  con_size[n] = " 25 80 " size of the specified console. Set from
#     #FORM# line in .fld
#----------------------------------------------------------------
function nl_error( level, text ) {
  system( "echo " FILENAME ":" NR lvlmsg[level] ": '" text "' >&2" )
  if ( level >= 2 ) {
	errorlevel = level
	exit
  }
}

function output_header( text ) {
  printf "\n"
  print "#----------------------------------------------------------------"
  print "# " text
  print "#----------------------------------------------------------------"
}

#----------------------------------------------------------------
# program is the name of the program
# background should be " &" if the program is supposed to run in
# the background, "" otherwise
#----------------------------------------------------------------
function output_app( program, background ) {
  if ( app[ program ] == "" )
	nl_error( 4, "Application not initialized" )
  printf "%s", program
  if ( ! match( opts[program], "-v" ) ) {
	if ( CMD[program] != "" && match( opts[program], "-p" ) && localring == "" ) {
	  printf " -v"
	} else {
	  printf " $_msgopts"
	}
  }
  if ( TM[program] != "" ) printf " $_dcopts"
  if ( CMD[program] != "" ) printf " $_cmdopts"
  if ( DISP[program] != "" ) printf "%s", DISP[program]
  if ( Statline[program] != "" )
	printf " %s", Statline[program]
  else if ( Statline["general"] != "" )
	printf " %s", Statline["general"]
  if ( opts[program] != "" ) printf " %s", opts[program]
  print background
  if ( background != "" )
	if ( bg_ids == 1 ) print "_bg_pids=\"$_bg_pids $!\""
}

BEGIN {
  lvlmsg[1] = " Warning"
  lvlmsg[2] = " Error"
  lvlmsg[3] = " Fatal"
  lvlmsg[4] = " Internal"
  batch_file_name = "interact"
  n_displays = 0
  n_exts = 0
  n_algos = 0
  scrno = -1
  lastcltscr = -1
  has_algos = 0
  n_screens = 1
  statusscreen = "non-empty"
  unbuffered = 0
  localring = ""
  autostart = 0
}
/^#FIELD#.*"%/ { if ( scrno < 0 ) nl_error( 4, "Got a #FIELD#" ) }
/^#FIELD#.*"%STATUS:/ {
  prog = $NF
  sub( "^\"%STATUS:", "", prog )
  sub( "\".*$", "", prog )
  Statline[ prog ] = "-o $_scr" scrno "," $3 "," $4 "," $5 ",$_attrs"
  next
}
/^#FIELD#.*"%STATUS"/ {
  Statline[ "general" ] = "-o $_scr" scrno "," $3 "," $4 "," $5 ",$_attrs"
  next
}
/^#FIELD#.*"%CLIENT"/ {
  if ( lastcltscr != scrno ) {
	if ( lastcltscr == -1 ) {
	  if ( scrno != 0 ) CltScreens = " -A $_scr" scrno
	} else CltScreens = CltScreens " -a $_scr" scrno
	lastcltscr = scrno
  }
  next
}
/^#FIELD#.*"%TMA:/ {
  prog = $NF
  sub( "\"%TMA:", "", prog )
  sub( "\"$", "", prog )
  pos = index( prog, ":" )
  if ( pos > 0 ) {
	partno = substr( prog, pos+1 )
	prog = substr( prog, 1, pos-1 )
  } else partno = 1
  tma_con[ prog, partno+0 ] = "$_scr" scrno
  tma_row[ prog, partno+0 ] = $3
  if ( partno+0 > tma_nparts[ prog ] )
	tma_nparts[ prog ] = partno+0
  next
}
/^#FORM#/ {
  if (scrno >= 0) con_size[scrno] = " " $2 " " $3 " "
  next
}
scrno >= 0 { next }
{ sub( "^[ \t]*", "" ) }
/^#/ { next }
/^[ /t]*$/ { next }
/^statusscreen/ { nl_error(1,"Obsolete use of statusscreen"); next }
/^autostart/ { autostart = 1; next }
/^memo/ {
  if ( NF > 1 ) log_file_name = $2
  memo = "yes"
  next
}
/^monoconfig/ { monoconfig = $2; next }
/^colorconfig/ { colorconfig = $2; next }
/^rtg/ {
  $1 = ""; sub( "^ *", "" )
  rtg = $0;
  next
}
/^batchfile/ { batch_file_name = $2; next }
/^display/ {
  if ( NF < 3 ) nl_error( 3, "display requires a screen name" )
  n_displays++
  displays[n_displays] = $2
  disp_screens[n_displays] = NF-2
  for ( i = 3; i <= NF; i++ ) {
	disp_con[n_displays,i-2] = n_screens
	disp_fld[n_displays,i-2] = $i
	# set the default size...
	con_size[ n_screens ] = " 25 80 "
	# Queue .fld for later processing
	ARGV[ARGC] = "scrno=" n_screens
	ARGC++
	ARGV[ARGC] = $i ".fld"
	ARGC++
	n_screens++
  }
  if ( app[ $2 ] == "" ) {
	app[ $2 ] = "yes"
	TM[ $2 ] = "yes"
  }
  next
}
/^extraction/ {
  prog = $2
  if ( app[ prog ] == "" ) {
	n_exts++
	exts[ n_exts ] = prog
	app[ prog ] = "yes"
  }
  TM[ prog ] = "yes"
  if ( opts[ prog ] == "" && NF > 2 ) {
	$1 = $2 = ""; sub( "^ *", "" )
	opts[ prog ] = $0
  }
  next
}
/^algorithm/ {
  prog = $2
  if ( app[ prog ] == "" ) {
	n_algos++
	algos[n_algos] = prog
	app[ prog ] = "yes"
  }
  TM[ prog ] = "yes"
  CMD[ prog ] = "yes"
  if ( opts[ prog ] == "" && NF > 2 ) {
	$1 = $2 = ""; sub( "^ *", "" )
	opts[ prog ] = $0
  }
  has_algos = 1
  next
}
/^playback/ {
  DG[ "name" ] = "rdr"
  DG[ "opts" ] = " -vc0 -d $*"
  localring = "PB"
  next
}
/^inetin/ {
  DG[ "name" ] = "Inetin"
  DG[ "opts" ] = " -vc0"
  localring = "IN"
  next
}
/^client/ {
  if ( client != "" )
	nl_error( 3, "Only one keyboard client allowed per script" )
  client = $2
  app[ client ] = "yes"
  CMD[ client ] = "yes"
  if ( opts[ client ] == "" && NF > 2 ) {
	$1 = $2 = ""; sub( "^ *", "" )
	opts[ client ] = $0
  }
  next
}
/^unbuffered/ {
  unbuffered = 1
  next
}
{ nl_error( 3, "Syntax Error: " $0 ) }
END {
  if ( errorlevel > 2 ) exit 1
  if (n_displays == 0 && n_exts == 0 && n_algos == 0 && client == "" )
	nl_error( 3, "No programs defined" )
  if ( client == "" && localring != "" ) {
	client = "PBclt"
	app[ client ] = "yes"
	CMD[ client ] = "yes"
  }
  if ( localring != "" ) {
	if ( log_file_name == "" )
	  log_file_name = "$RemEx.$LocalRing.log";
	unbuffered = 1
  }
  if ( log_file_name == "" )
	log_file_name = "$Experiment.log";
  if ( memo == "yes" && client == "" )
	nl_error( 3, "memo requires a client" )
  print "#! /bin/sh"
  print "#__USAGE"
  print "#%C"
  print "#	Starts Instrument operation, including"
  if ( localring != "" ) {
	print "#	  " DG["name"]
  } else {
	print "#	  batchfile " batch_file_name
  }
  for (i = 1; i <= n_displays; i++) {
	print "#	  display " displays[i]
  }
  for (i = 1; i <= n_exts; i++) {
	print "#	  extraction " exts[i]
  }
  for (i = 1; i <= n_algos; i++) {
	print "#	  algorithm " algos[i]
  }
  if ( client != "" ) print "#	  client " client
  if ( memo == "yes" ) print "#	  command log"
  if ( rtg != "" ) print "#	  rtg"
  if ( localring == "" ) {
	print "#%C	stop"
	print "#	Terminates Instrument Operations"
	print "#%C	not"
	print "#	Prevents Instrument Operation on power up by invoking"
	print "#	pick_file /dev/null"
	print "#%C	wait"
	print "#	Delays starting GSE until flight node has begun"
	print "#	the startup process. This verifies that the flight"
	print "#	system operates in flight configuration."
  }

  output_header( "Perform Common Initializations" )
  print "boilerplate=mkdoit2.sh"
  print "for dir in /usr/local/lib/src /usr/local/bin /usr/lib; do"
  print "  if [ -f $dir/$boilerplate ]; then"
  print "	boilerplate=$dir/$boilerplate"
  print "	break"
  print "  fi"
  print "done"
  print "[ ! -f $boilerplate ] && {"
  print "  echo Cannot locate boilerplate script $boilerplate >&2"
  print "  exit 1"
  print "}"
  # if ( client == "PBclt" ) print "PlayBack=yes"
  print "LocalRing=\"" localring "\""
  print ". $boilerplate"
  print "[ -z \"\$TMBINDIR\" ] && TMBINDIR='.'"

  #----------------------------------------------------------------
  # Now we need to collect the required consoles
  #----------------------------------------------------------------
  if ( n_screens == 1 ) n_screens = 2
  if ( memo == "yes" ) n_screens++
  if ( n_screens > 1 ) {
	output_header( "Allocate Screens" )
	printf "typeset"
	for (i = 1; i < n_screens; i++) printf " _scr" i
	printf "\n"

	print "if [ -n \"$_scrdefs\" ]; then"
	printf "%s", "  eval `getcon $condev"
	for (i = 1; i < n_screens; i++) printf " _scr" i
	print "`"
	print "elif [ $winrunning = yes ]; then"
	print "  exec on -t $condev $0 -W $*"
	print "else"
	if ( n_screens > 2 ) {
	  printf "%s", "  eval `getcon $condev"
	  for (i = 2; i < n_screens; i++) printf " _scr" i
	  print "`"
	}
	print "  _scr1=$_scr0"
	print "fi"

	printf "%s", "[ -z \"$_scr" n_screens-1 "\" ] && "
	print "nl_error Unable to allocate enough screens"
  }

  print "[ $winrunning = yes ] && {"
  print "  winsetsize $_scr0 8 45 `basename $0`"
  print "  echo Allocating Screens > $_scr0"
  for ( i = 1; i <= n_displays; i++ ) {
	n_scrs = disp_screens[i]
	for ( j = 1; j <= n_scrs; j++ ) {
	  con_no = disp_con[i,j]
	  console = "$_scr" con_no
	  print "  winsetsize " console con_size[con_no] disp_fld[i,j]
	  print "  echo \"\\033/2t \\r\\c\" > " console
	}
  }
  if ( memo == "yes" ) {
	print "  escq=\"\\033\\\"\""
	print "  logf=\"" log_file_name "\\\"\""
	printf "  %s", "echo \"${escq}t$logf${escq}i$logf"
	print  "${escq}p$logf\\033/2t \\r\\c\" >$_scr" n_screens-1
  }
  print "}"
  
  #----------------------------------------------------------------
  # bg_procs is true if there are background processes
  # bg_ids is true if we need to keep track of their pids
  #----------------------------------------------------------------
  if ( n_displays > 0 || n_exts > 0 || n_algos > 0 || client == "PBclt" ) {
	bg_procs = 1
  } else bg_procs = 0

  if (client != "" && bg_procs && localring == "" ) {
	bg_ids = 1
	print "typeset _bg_pids='-p'"
  } else bg_ids = 0

  output_header( "Instrument Startup Sequence" )
  if ( localring != "" ) {
	print "[ -z \"$_doit_basedir\" ] && _doit_basedir=."
	print "memo -vy -e $_doit_basedir/" log_file_name " &"
	print "namewait -p $! memo || nl_error Error launching memo"
	# print "_bg_pids=\"$_bg_pids $!\""
	if ( autostart ) {
	  DG["opts"] = DG["opts"] " -n " n_displays+n_exts+n_algos+1
	}
	print DG["name"] DG["opts"] " &"
	print "namewait -p $! -g dg || nl_error Error launching " DG["name"]
	# print "_bg_pids=\"$_bg_pids $!\""
  } else {
	print "if [ -n \"$wait_for_node\" ]; then"
	print "  echo Waiting for Flight Node to Boot"
	print "  [ $winrunning = yes ] && echo \"\\033/5t\\c\""
	print "  namewait -n0 dg"
	print "fi"
	print "echo Waiting for pick_file"
	print "[ $winrunning = yes ] && echo \"\\033/5t\\c\""
	printf "%s", "FlightNode=`pick_file -n " batch_file_name

	printf "%s", " 2> $_scr0"
	print "`"
	print "[ -n \"$FlightNode\" ] || nl_error pick_file returned an error"
  }
  printf "\n"
  print "_msgopts=\"-v -c$FlightNode\""
  if ( unbuffered ) {
	print "_dcopts=\"\""
  } else {
	print "_dcopts=\"-b$FlightNode -i1\""
  }
  print "_cmdopts=\"-C$FlightNode\""

  if ( memo == "yes" ) {
	# print "\nwinsetsize $_scr" n_screens-1 " 25 80 " log_file_name
	output_header( "Startup Memo Log Window" )
	printf "%s", "on -t $_scr" n_screens-1
	printf "%s", " less +F "
	if ( localring == "" ) printf "%s", "//$FlightNode$HomeDir/"
	else printf "$_doit_basedir/"
	print log_file_name
	print "[ $winrunning = yes ] && echo \"\\033/1t\\c\" > $_scr" n_screens-1
	print "getcon -r $_scr" n_screens-1
  }

  if ( rtg != "" ) {
	print "start_rtg " rtg "\n"
  }
  
  if ( n_displays > 0 ) {
	if ( colorconfig != "" || monoconfig != "" )
	  print "typeset _cfgfile"
	print "if [ -z \"$MONOCHROME\" ]; then"
	print "  _attrs=02,06,04,05"
	if ( colorconfig != "" ) print "  _cfgfile=" colorconfig
	if ( monoconfig != "" ) {
	  print "else"
	  print "  _cfgfile=" monoconfig
	}
	print "fi\n"
  }

  if ( unbuffered == 0 && (n_displays > 0 || n_exts > 0 || n_algos > 0 ) ) {
	print "echo Waiting for Data Buffer"
	print "[ $winrunning = yes ] && echo \"\\033/5t\\c\""
	print "namewait -n$FlightNode db\n"
  }
  
  if ( client != "PBclt" && ( has_algos > 0 || client != "" ) ) {
	print "echo Waiting for Command Interpreter"
	print "[ $winrunning = yes ] && echo \"\\033/5t\\c\""
	print "namewait -n$FlightNode cmdinterp\n"
  }

  if ( localring != "" ) {
	output_header( "Data Regulator (Must be first after DG)" )
	print "PBreg $_msgopts $_dcopts -h Reg &"
	# print "_bg_pids=\"$_bg_pids $!\""
	print "namewait -p $! PBreg || nl_error Error launching PBreg"
	printf "\n"
  }

  if ( n_displays > 0 )
	output_header( "Display Programs:" )
  #----------------------------------------------------------------
  # Determine display options for each display program
  #----------------------------------------------------------------
  for ( i = 1; i <= n_displays; i++ ) {
	prog = displays[i]
	part_no = 1 # Next partition to consider for display
	for ( j = 1; j <= disp_screens[i]+0; j++ ) {
	  if ( j != 1 || disp_con[i,j] != 0 ) {
		if ( j == 1 ) opt = "-A"
		else opt = "-a"
		DISP[ prog ] = DISP[ prog ] " " opt " $_scr" disp_con[i,j]
	  }
	  # Now check to see if we have partition status for this prog
	  while ( part_no <= tma_nparts[ prog ]+0 ) {
		part_con = tma_con[ prog, part_no+0 ]
		if ( part_con != "" ) {
		  if ( part_con < disp_con[i,j] ) {
			# report error cannot display partition part_no
			nl_error( 2, "Cannot display " prog " partition " part_no )
			DISP[ prog ] = DISP[ prog ] " -r -1"
			part_no++
		  } else if ( part_con == disp_con[i,j] ) {
			DISP[ prog ] = DISP[ prog ] " -r " tma_row[ prog, part_no ]
			part_no++
		  } else break
		}
	  }
	}
  }
  
  #----------------------------------------------------------------
  # display each screen
  #----------------------------------------------------------------
  for ( i = 1; i <= n_displays; i++ ) {
	n_scrs = disp_screens[i]
	for ( j = 1; j <= n_scrs; j++ ) {
	  con_no = disp_con[i,j]
	  console = "$_scr" con_no
	  if ( console == "$_scr0" ) scr = ""
	  else {
		scr = " > " console " < " console
		scr = scr "; stty +opost < " console
	  }
	  printf "%s", "scrpaint $_msgopts \$TMBINDIR/" disp_fld[i,j] ".fld"
	  if ( colorconfig != "" || monoconfig != "" )
		printf " \$TMBINDIR/$_cfgfile"
	  print scr
	  print "[ $winrunning = yes ] && echo \"\\033/1t\\c\" > " console
	}
	output_app( displays[ i ], " &" )
  }

  if ( n_screens > 1 ) {
	print "[ $winrunning = yes ] && echo \"\\033/2t\\c\""
  }
  if ( n_exts > 0 ) {
	output_header( "Extraction Programs:" )
	for ( i = 1; i <= n_exts; i++ )
	  output_app( exts[ i ], " &" )
  }
  
  if ( n_algos > 0 ) {
	output_header( "Algorithms:" )

	for ( i = 1; i <= n_algos; i++ ) {
	  prog = algos[ i ]
	  curscr = ""
	  for ( part_no = 1; part_no <= tma_nparts[ prog ] + 0; part_no++ ) {
		part_con = tma_con[ prog, part_no+0 ]
		if ( part_con == "" ) {
		  if ( curscr == "" ) curscr = "$_scr0"
		  DISP[ prog ] = DISP[ prog ] " -r -1"
		} else {
		  if ( part_con != curscr ) {
			if ( curscr == "" ) {
			  if ( part_con != "$_scr0" )
				DISP[ prog ] = DISP[ prog ] " -A " part_con
			} else DISP[ prog ] = DISP[ prog ] " -a " part_con
			curscr = part_con
		  }
		  DISP[ prog ] = DISP[ prog ] " -r " tma_row[ prog, part_no ]
		}
	  }
	  output_app( prog, " &" )
	}
  }

  if ( client != "" ) {
	output_header( "Keyboard Client:" )
	if ( lastcltscr != -1 )
	  DISP[ client ] = DISP[ client ] CltScreens
	else {
	  j = n_screens
	  if ( memo == "yes" ) j--
	  if ( 1 < j ) {
	    DISP[ client ] = DISP[ client ] " -A $_scr1"
		for ( i = 2; i < j; i++ )
		  DISP[ client ] = DISP[ client ] " -a $_scr" i
	  }
	}
	output_app( client, "" )

	if ( memo == "yes" )
	  print "slay -t /${_scr" n_screens-1 "#//*/} less"
	if ( localring != "" )
	  print "memo -vk0"
  }

  if ( bg_procs == 1 ) {
	print "if [ $winrunning = yes ]; then"
	print "  _scrs=\"\""
	print "  echo \"\\033/1t\\033/5tShutting Down...\""
	print "else"
	printf "%s", "  _scrs=\""
	for ( i = 0; i < n_screens; i++ ) {
	  printf " $_scr" i
	}
	print "\""
	print "fi"
	printf "%s", "exec parent -qvn"
	if ( localring != "" || bg_ids == 1 ) printf "t3"
	if ( bg_ids == 1 ) printf "%s", " \"$_bg_pids\""
	print " $_scrs"
  }
}
