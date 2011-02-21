# converts .slp format files to .cmd format
# The .slp format looks essentially like this:
#
#  ; This is a comment
#  Command_Set = 'A'
#  proxy etalon {
#    O: Drive Etalon Online
#    _: Drive Etalon Offline
#  }
#
BEGIN {
  command_set="A"
  print "%{"
  print "  #ifdef SERVER"
  print "\t#define SERVER_INIT"
  print "\tvoid cis_initialize(void) {"
  print "\t  int ret = 0;"
}
FILENAME != this_file {
  this_file = FILENAME;
  this_prox = 0;
}
/^[ \t]*;/ { # comment
  sub("^[ \t]*;", "\t  /*")
  print $0 " */"
  next
}
/^[ \t]*[Cc]ommand_[Ss]et/ {
  if (match($0, "'[A-J]'")) command_set=substr($0, RSTART+1, 1)
  next
}
/^[ \t]*[Pp]roxy/ {next}
/^[ \t]*.:/ {
  nprox++
  this_prox++
  proxy[nprox]="SOLDRV_PROXY_" command_set ", " this_prox;
  sub("^[ \t]*.: *", "\t  ret |= solp_init(" proxy[nprox] ", \"")
  sub("[ \t]*$", "");
  print $0 "\\n\");"
  next
}
/^[ \t]*\}/ {next}
{ system("echo >&2 slp2cmd: Syntax Error in file " FILENAME " at line " FNR )
  system("echo >&2 \"--> " $0 "\"")
  syntaxerr=1
  exit 1
}
END {
  if (syntaxerr!=1) {
	print "\t  if (ret) exit(1);"
	print "\t}"
	print "\tvoid cis_terminate(void) {"
	for (i = 1; i <= nprox; i++)
	  print "\t  Soldrv_reset_proxy(" proxy[i] ");"
	print "\t}"
	print "  #endif\n%}"
  }
}
