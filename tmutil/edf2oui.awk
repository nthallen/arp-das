# edf2oui.awk Converts .edf files to .oui for extra usage
# $Log$
# Revision 1.2  2012/03/20 18:49:59  ntallen
# Changes to support CSV output
#
# Revision 1.1  2008/09/22 13:36:29  ntallen
# edf tools for QNX6
#
# Revision 1.2  1998/01/08 22:01:07  nort
# Eliminate a trivial difference
#
# Revision 1.1  1996/01/05  14:45:43  nort
# Initial revision
#
#
# spreadsheet deleteme 6
#  1 O3Ref %6.0lf Ct24_Double
#
/^ *(spreadsheet|csv|json) / {
  if (printed == 0) {
    print "<package> edfoui"
    printed = 1
  }
  print "<unsort>\n<BLANK>"
  sps = $2
}
/^ *spreadsheet / {
  print "Spreadsheet " sps ", " $3 " columns:"
}
/^ *csv / {
  print "CSV " sps ", " $3 " columns:"
}
/^ *json / {
  print "JSON " sps ", " $3 " columns:"
}
/^[ \t]*[0-9]/ {
  printf "  %-13s[%2d] = %s\n", sps, $1, $2
}
