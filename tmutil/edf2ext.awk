# edf2ext.awk Converts .edf files to .ext for TMC input.
# $Log$
# Revision 1.5  2010/08/13 18:10:47  ntallen
# Typo
#
# Revision 1.4  2010/08/13 18:00:24  ntallen
# check ss_insert_value() return code.
#
# Revision 1.3  2009/10/05 01:10:21  ntallen
# const char * remediation
#
# Revision 1.2  2008/09/22 16:00:30  ntallen
# Updates to appgen for extractions
# mkinp and tomat utilities
#
# Revision 1.1  2008/09/22 13:36:29  ntallen
# edf tools for QNX6
#
# Revision 1.6  1998/01/08 21:59:27  nort
# To eliminate an errant difference
# .,
#
# Revision 1.5  1997/01/09  23:17:05  nort
# Added support for conditions
#
# Revision 1.4  1995/10/27  18:52:51  nort
# made nl_error() msg(), since it's already included for
#
# Revision 1.3  1995/10/27  17:12:28  nort
# Added support for appending to existing spreadsheets
#
# Revision 1.1  1993/05/28  20:06:11  nort
# Initial revision
#
# spreadsheet deleteme 6
#  1 O3Ref %6.0lf Ct24_Double
#
BEGIN { rv = 0 }
/^ *spreadsheet/ {
  if (written == 1) nsps++
  else {
    print "%{ /* edf2ext.awk reading " FILENAME " */"
    print "  #include <stdlib.h>"
    print "  #include <errno.h>"
    print "  #include \"ssp.h\""
    print "  #include \"snafuerr.h\""
    print "  #include \"msg.h\""
    print "  #include \"tmctime.h\""
    printf "\n"
    print "  #define Ct24_Long(x) (0xFFFFFF & *(TMLONG *)&x)"
    print "  #define Ct24_Double(x) (double)Ct24_Long(x)"
    print "  #define To_Double(x) (double)(x)"
    print "  #define EXTRACTION_INIT initialize()"
    print "  #define EXTRACTION_TERM terminate()"
    print "  #define ALL_SPSS"
    printf "\n"
    print "  static double ext_delta = 0.;"
    printf "\n"
    print "  static sps_ptr edf_ss_open( const char *name, int width ) {"
    print "    sps_ptr ss;"
    printf "\n"
    print "    ss = ss_open( name );"
    print "    if ( ss_error( ss ) ) {"
    print "      ss = ss_create( name, SPT_INCREASING, width, 0 );"
    print "      if ( ss_error( ss ) )"
    print "        msg( 3, \"Unable to create spreadsheet %s\", name );"
    print "      errno = 0;"
    print "      msg( 0, \"Creating spreadsheet %s.sps\", name );"
    print "    } else if ( ss_width( ss ) != width )"
    print "      msg( 3,"
    print "        \"Existing spreadsheet %s.sps not of width %d\", width );"
    print "    else msg( 0, \"Appending to spreadsheet %s.sps\", name );"
    print "    return ss;"
    print "  }"
    printf "\n"
    print "  static void edf_ss_insert_val( sps_ptr ssp, double V, double delta ) {"
    print "    int rv = ss_insert_value( ssp, V, delta );"
    print "    if (rv != 0 ) {"
    print "      ss_close_all();"
    print "      if ( rv == SFU_SPDSHT_FULL ) nl_error( 3, \"Spreadsheet full\");"
    print "      else nl_error( 3, \"SNAFU Error %d on insert\", rv );"
    print "    }"
    print "  }"
    printf "\n"
    written = 1;
    nsps = 0;
  }
  sps[nsps] = $2
  ncols[nsps] = $3
  cond[nsps] = ""
  if (NF > 3 && $4 == "separate") sep[nsps] = "y"
  next
}
/^[ \t]*condition/ {
  cnd = $0
  sub( "^[ \t]*condition[ \t]*", "", cnd )
  cond[nsps] = cnd " "
  next
}
/^[ \t]*[0-9]/ {
  datum[nsps,$1] = $2
  if (NF >= 3) datfmt[nsps,$1] = $3
  else datfmt[nsps,$1] = "%9.2e"
  if (NF >= 4) datcnv[nsps,$1] = $4
  else datcnv[nsps,$1] = "convert"
  next
}
/init_only/ { init_only = "yes"; next }
/^[ \t]*[#;%]/ { next }
/^[ \t]*$/ { next }
{ system( "echo " FILENAME ":" NR " Syntax error >&2" )
  exit( rv = 1 )
}
END {
  if ( rv ) { exit(1); }
  # print the spreadsheet declarations
  for (i = 0; i <= nsps; i++)
    print "  sps_ptr " sps[i] ";"

  # print the initializations
  print "  void initialize(void) {"
  print "    {"
  print "      char *s;"
  print "      s = getenv(\"EXT_DELTA\");"
  print "      if (s != NULL) {"
  print "        ext_delta=atof(s);"
  print "        msg(MSG, \"Using Time Delta of %lf\", ext_delta);"
  print "      }"
  print "    }"
  for (i = 0; i <= nsps; i++) {
    print "    " sps[i] " = edf_ss_open( \"" sps[i] "\", " ncols[i] " );"
    print "    ss_set_column(" sps[i] ", 0, \"%14.11lt\", \"Time\");"
    for (j = 1; j < ncols[i]; j++) {
      if (datfmt[i,j] == "") datfmt[i,j] = "%9.2e"
      printf "    ss_set_column(" sps[i] ", " j ", "
      print "\"" datfmt[i,j] "\", \"" datum[i,j] "\");"
    }
  }
  print "  }"
  
  # print the terminations
  print "  void terminate(void) {"
  for (i = 0; i <= nsps; i++) {
    print "    ss_close(" sps[i] ");"
  }
  print "  }"
  print "%}"

  # print the extraction statements
  if (init_only != "yes") {
    for (i = 0; i <= nsps; i++) {
      k = 0;
      for (j = 1; j < ncols[i]; j++) {
        if (datum[i,j] != "") {
          if (k > 0 && sep[i] == "y") print "}"
          if (k == 0 || sep[i] == "y") {
            print cond[i] "{"
            print "  edf_ss_insert_val(" sps[i] ", dtime()+ext_delta, 0);"
          }
          printf "  ss_set(" sps[i] ", " j ", "
          print datcnv[i,j] "(", datum[i,j] "));"
          k++;
        }
      }
      print "}"
    }
  }
}
