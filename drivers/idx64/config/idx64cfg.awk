BEGIN {
  lvlmsg[0] = ""
  lvlmsg[1] = "Warning: "
  lvlmsg[2] = "Error: "
  lvlmsg[3] = "Fatal: "
  lvlmsg[4] = "Internal: "
  errorlevel = 0
  max_channel = -1
  r_bits[ 53 ] = 0 + 1
  r_bits[ 80 ] = 1 + 1
  r_bits[ 107 ] = 2 + 1
  r_bits[ 160 ] = 3 + 1
  r_bits[ 267 ] = 4 + 1
  r_bits[ 400 ] = 5 + 1
  r_bits[ 533 ] = 6 + 1
  r_bits[ 800 ] = 7 + 1
  r_bits[ 1067 ] = 8 + 1
  r_bits[ 1600 ] = 9 + 1
  r_bits[ 2133 ] = 10 + 1
  r_bits[ 3200 ] = 11 + 1
  r_bits[ 5033 ] = 12 + 1
  r_bits[ 8000 ] = 13 + 1
  r_bits[ 10667 ] = 14 + 1
  r_bits[ 16000 ] = 15 + 1
  basetmc = 0
  coltmc = 0
  flagtmc = 0
  cmdstr = 0
  drivecmd = 0
  scanstats = 0
  drivestats = 0
  IXStt = 0
}

function set_value( type, kw, chan, val ) {
  if ( values[ kw, type, chan ] != "" )
	nl_error( 2, "Redefinition of " kw " " type " for channel " chan )
  else values[ kw, type, chan ] = val
}
function nl_error( level, msg ) {
  if ( level > 4 ) level = 4
  else if ( level < 0 ) level = 0
  system( "echo \"" FILENAME "(" NR "): " lvlmsg[level] msg "\" >&2" )
  if ( level > 2 ) {
	errorlevel = 1
	exit
  }
}

/^ *#/ { next }
/^[ \t]*$/ { next }
/^ *[Ii]ndexer +[Ss]tatus +[0-9/]+ +Hz +[A-Za-z_]/ {
  set_value( "IXStt", "mnemonic", 0, $5 )
  set_value( "IXStt", "rate", 0, $3 )
  IXStt = 1
  next
}
/^ *[Cc]hannel +[0-9]+/ {
  curr_chan = $2
  if ( chan_entry[ curr_chan ] != "" )
	nl_error( 1, "Duplicate channel entry" )
  chan_entry[ curr_chan ] = "yes"
  name = $0
  sub( "^ *[A-Za-z]+ +[0-9]+ *", "", name )
  chan_name[ curr_chan ] = name
  if ( curr_chan > max_channel ) max_channel = curr_chan
  next
}
/^ *[Pp]osition +[0-9/]+ +Hz +[A-Za-z_]/ {
  set_value( "rate", "position", curr_chan, $2 )
  set_value( "mnemonic", "position", curr_chan, $4 )
  next
}
/^ *[Ss]tatus +[0-9/]+ +Hz +[A-Za-z_]/ {
  set_value( "rate", "status", curr_chan, $2 )
  set_value( "mnemonic", "status", curr_chan, $4 )
  sfld = 5
  while ( sfld <= NF ) {
    if ( $sfld == "Diagram" ) {
      drivestats = 1
      set_value( "Diagram", "status", curr_chan, $(sfld+1) )
    } else if ( $sfld == "KillStat" ) {
      killstats = 1
      set_value( "KillStat", "status", curr_chan, $(sfld+1) )
    }
    sfld = sfld+2
  }
  next
}
/^ *[Ss]can[Ss]tat +[A-Za-z][A-Za-z0-9_]* *$/ {
  set_value( "mnemonic", "scanstat", curr_chan, $2 )
  scanstats = 1
  next
}
/^ *[Ss]peed +[0-9]+ +Hz *$/ {
  rbits = r_bits[ $2 ]
  if ( rbits > 0 )
	set_value( "rate", "drive", curr_chan, rbits )
  else nl_error( 3, "Drive Rate " $2 " Hz is not supported" )
  next
}
/^ *[Dd]isable +[Zz]eroref/ {
  set_value( "flag", "zeroref", curr_chan, 1 )
  next
}
/^ *[Ss]wap +[Ll]imits/ {
  set_value( "flag", "swap_limits", curr_chan, 1 )
  next
}
/^ *[Ii]nvert +[Ss]tep/ {
  set_value( "flag", "inv_step", curr_chan, 1 )
  next
}
/^ *[Ii]nvert +[Dd]irection/ {
  set_value( "flag", "inv_dir", curr_chan, 1 )
  next
}
/^ *[Ii]nvert +[Rr]un/ {
  set_value( "flag", "inv_run", curr_chan, 1 )
  next
}
/^ *[Ss]can *$/ {
  set_value( "flag", "scan", curr_chan, 1 )
  next
}
/^ *[Cc]hop *$/ {
  set_value( "flag", "chop", curr_chan, 1 )
  next
}
{ nl_error( 3, "Syntax Error" ) }

END {
  if ( errorlevel != 0 ) exit 1

  #----------------------------------------------------------------
  # Determine Status Distribution
  #----------------------------------------------------------------
  startbit = 0
  flagword = 0
  for ( i = 0; i <= max_channel; i++ ) {
	rbits = 0
	if ( values["scan", "flag", i ] ) rbits += 1
	if ( values["chop", "flag", i ] ) rbits += 2
	nbits[i] = rbits
	if ( startbit + rbits > 16 ) {
	  flagword++
	  startbit = 0
	}
	stbit[i] = startbit
	flwd[i] = flagword
	startbit += rbits
  }
  if ( startbit == 0 && IXStt != 0 ) {
	nl_error( 3, "Indexer Status defined, but empty" );
  }
  if ( startbit > 0 && IXStt == 0 ) {
	if ( basetmc ) {
	  nl_error( 1, "Indexer Status undefined, using default" );
	}
	set_value( "IXStt", "mnemonic", 0, "IXStt" )
	set_value( "IXStt", "rate", 0, "4" )
  }

  #----------------------------------------------------------------
  # Generate TMC definitions
  #----------------------------------------------------------------
  if ( basetmc ) {
	print "TM typedef unsigned short IndxrPos {"
	print "  text \"%5d\";"
	print "  collect x = sbwa(x.address);"
	print "}"
	print "TM typedef unsigned char IndxrStat {"
	print "  text \"%08b\";"
	print "  collect x = sbba(x.address);"
	print "}"
	for ( i = 0; i <= max_channel; i++ ) {
	  bdno = int( i/6 )
	  chno = i%6
	  bdbase = 2568 + ( bdno * 64 )
	  chbase = bdbase + ( chno * 8 )
	  if ( values[ "position", "mnemonic", i ] != "" ) {
		printf "TM %s Hz", values[ "position", "rate", i ]
		printf " IndxrPos  %s; ", values[ "position", "mnemonic", i ]
		printf " Address %s ", values[ "position", "mnemonic", i ]
		printf "0x%03X;\n", chbase + 4
	  }
	  if ( values[ "status", "mnemonic", i ] != "" ) {
		printf "TM %s Hz", values[ "status", "rate", i ]
		printf " IndxrStat %s; ", values[ "status", "mnemonic", i ]
		printf " Address %s ", values[ "status", "mnemonic", i ]
		printf "0x%03X;\n", chbase + 6
	  }
	}
  }
  if ( flagword > 0 || startbit > 0 ) {
	if ( flagword > 0 ) subs = "[" flagword+1 "]"
	else subs = ""
	if ( coltmc ) {
	  print "%{"
	  print "  #include <sys/types.h>"
	  print "  #include \"subbus.h\""
	  print "  pid_t IndxrProxy;"
	  print "%}"
	  print "TM \"proxy\" IndxrProxy 2;"
	  print "IndxrPos Idx64" subs ";"
	  print "TM \"Receive\" Idx64 0;"
	}
	if ( basetmc ) {
	  printf "TM " values["rate","IXStt",0] " Hz IndxrPos  "
	  print values["mnemonic","IXStt",0] subs ";"
	}
	if ( coltmc ) {
	  print "Collect " values["mnemonic","IXStt",0] " {"
	  if ( flagword > 0 ) {
		print "  int i;"
		print "  for (i=0;i<" flagword+1 ";i++) {"
		print "    " values["mnemonic","IXStt",0] "[i] = Idx64[i];"
		print "  }"
	  } else print "  " values["mnemonic","IXStt",0] " = Idx64;"
	  print "  if (IndxrProxy!=0) Trigger(IndxrProxy);"
	  print "}"
	}
  }


  #----------------------------------------------------------------
  # Generate Configuration String
  #----------------------------------------------------------------
  if ( cfgstr ) {
	for ( i = 0; i <= max_channel; i++ ) {
	  if ( i > 0 ) printf ":"
	  cfgval=0
	  if ( values["swap_limits","flag",i] ) cfgval += 1
	  if ( values["zeroref","flag",i] ) cfgval += 2
	  if ( values["inv_step","flag",i] ) cfgval += 4
	  if ( values["inv_dir", "flag",i] ) cfgval += 8
	  if ( values["inv_run", "flag",i] ) cfgval += 16
	  if ( cfgval != 0 ) cfgval += 32
	  rbits = values["drive","rate",i]
	  if ( cfgval != 0 && rbits == 0 ) rbits = 13
	  if ( rbits > 0 ) {
		cfgval += ( rbits - 1 ) * 256
		if ( cfgval != 3104 ) printf "%03X", cfgval
	  }
	  if ( nbits[i] != 0 ) printf ",%d", nbits[i]
	}
	printf "\n"
  }

  #----------------------------------------------------------------
  # Generate derived TMC definitions for scanstats
  #----------------------------------------------------------------
  if ( flagtmc ) {
	if ( drivestats ) {
	  print "%{"
	  print "  char *IxDrive_text[] = {"
	  print "\t\"  <  \","
	  print "\t\"| <  \","
	  print "\t\"  < |\","
	  print "\t\"| < |\","
	  print "\t\"  >  \","
	  print "\t\"| >  \","
	  print "\t\"  > |\","
	  print "\t\"| > |\","
	  print "\t\"<----\",  /* driving in */"
	  print "\t\"|<---\",  /* driving in against in limit */"
	  print "\t\"<---|\",  /* driving in from out limit */"
	  print "\t\"|<--|\",  /* driving in both limits. Broke */"
	  print "\t\"---->\",  /* driving out */"
	  print "\t\"|--->\",  /* driving out from in limit */"
	  print "\t\"--->|\",  /* driving out agains out limit */"
	  print "\t\"|-->|\"   /* driving out both limits. Broke */"
	  print "  };"
	  print "%}"
	  print "TM typedef unsigned char IxDriveStat { text \"%5d\" IxDrive_text[]; }"
	  for ( i = 0; i <= max_channel; i++ ) {
		dmnem = values["status","Diagram",i]
		mnem = values["status", "mnemonic",i]
		if ( dmnem != "" && mnem != "" ) {
		  printf "IxDriveStat " dmnem "; invalidate " dmnem ";"
		  print " { " dmnem " = " mnem " & 0xF; Validate " dmnem "; }"
		}
	  }
	}
	if ( killstats ) {
	  print "%{"
	  print "  char *IxKillStat_text[] = {"
	  print "    \"----\","
	  print "    \"---A\","
	  print "    \"--B-\","
	  print "    \"--BA\","
	  print "    \"-Z--\","
	  print "    \"-Z-A\","
	  print "    \"-ZB-\","
	  print "    \"-ZBA\","
	  print "    \"C---\","
	  print "    \"C--A\","
	  print "    \"C-B-\","
	  print "    \"C-BA\","
	  print "    \"CZ--\","
	  print "    \"CZ-A\","
	  print "    \"CZB-\","
	  print "    \"CZBA\""
	  print "  };"
	  print "%}"
	  print "TM typedef unsigned char IxKillStat { text \"%4d\" IxKillStat_text[]; }"
	  for ( i = 0; i <= max_channel; i++ ) {
		dmnem = values["status","KillStat",i]
		mnem = values["status", "mnemonic",i]
		if ( dmnem != "" && mnem != "" ) {
		  printf "IxKillStat " dmnem "; invalidate " dmnem ";"
		  print " { " dmnem " = (" mnem " >> 4) & 0xF; Validate " dmnem "; }"
		}
	  }
	}
	if ( scanstats ) {
	  print "%{"
	  print "  char *IdxFlgTxt[8] = {"
	  print "\t\"    \","
	  print "\t\"Scan\","
	  print "\t\"On  \","
	  print "\t\"S/On\","
	  print "\t\"Off \","
	  print "\t\"S/Of\","
	  print "\t\"Alt \","
	  print "\t\"S/Al\""
	  print "  };"
	  print "%}"
	  print "TM typedef unsigned short IndxrFlag {"
	  print "  text \"%4d\" IdxFlgTxt[];"
	  print "}\n"

	  for ( i = 0; i <= max_channel; i++ ) {
		mnemonic = values["scanstat","mnemonic",i]
		if ( mnemonic != "" ) {
		  print "IndxrFlag " mnemonic "; invalidate " mnemonic ";"
		  if ( nbits[i] != 0 ) {
			sbit = stbit[i]
			mask = (2 ^ nbits[i]) - 1
			if ( nbits[i] == 2 ) {
			  sbit--
			  mask *= 2
			}
			printf "{ %s = ( " values["mnemonic","IXStt",0], mnemonic
			if ( flagword > 0 ) printf "[%d]", flwd[i]
			if ( sbit > 0 ) printf " >> %d", sbit
			else if ( sbit < 0 ) printf " << 1"
			printf " ) & %d; validate %s; }\n", mask, mnemonic
		  }
		}
	  }
	}
  }

  #----------------------------------------------------------------
  # Generate Drive Definitions for .cmd
  #----------------------------------------------------------------
  if ( drivecmd ) {
	print "&drive"
	for ( i = 0; i <= max_channel; i++ ) {
	  if ( chan_name[i] != "" )
		print "\t: " chan_name[i] " { $0 = " i "; }"
	}
	print "\t;"
  }
}
