# Converts .slp format files to .sol format
# The .slp format looks essentially like this:
#
#  ; This is a comment
#  Command_Set = 'A'
#  proxy etalon {
#    O: Drive Etalon Online
#    _: Drive Etalon Offline
#  }
#
/^[ \t]*;/ { print; next }
/^[ \t]*[Cc]ommand_[Ss]et/ { print; next }
/^[ \t]*[Pp]roxy/ { print; next }
/^[ \t]*.:/ {
  match($0, "^[ \t]*.:[ \t]*")
  proxy[++nprox] = substr($0, RSTART+RLENGTH)
  match($0, "^[ \t]*.:")
  print substr($0, RSTART, RLENGTH) nprox
  next
}
/^[ \t]*\}/ { print; next }
{ system("echo >&2 slp2cmd: Syntax Error in file " FILENAME " at line " FNR )
  system("echo >&2 \"--> " $0 "\"")
  syntaxerr=1
  exit 1
}
END {
  if (syntaxerr!=1) {
	for (i = 1; i <= nprox; i++) {
	  print "; proxy " i ": " proxy[i] 
	}
  }
}
