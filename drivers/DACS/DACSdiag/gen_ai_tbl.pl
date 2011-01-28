#! /usr/bin/perl -w
open( STDOUT, ">", "ana_in.tbl.new" ) || die;
print "AD {\n";
for my $bank ( 0, 1 ) {
  print "  HBox { +-; \"Bank $bank\"; -+ };\n  \"\"";
  print map( "  >\"$_\"", (0 .. 7) ), ";\n";
  for my $row (0 .. 7 ) {
    print "  \"$row:\"";
    for my $col (0 .. 7 ) {
      my $naddr = 0xC00 + $row*32 + $bank*16 + $col*2;
      my $addr = sprintf( "%03X", $naddr );
      my $chan = "AI$addr";
      print " ($chan,7)";
      print "\n     " if $col == 3;
    }
    print ";\n";
  }
}
print "}\n\n{\n",
  "  HBox { |+; [AD]; |+ };\n  -;\n    >{ Time: (flttime,8) MFCtr: (MFCtr,5) }\n}\n";

close STDOUT || warn "Error closing STDOUT\n";
system( "phtable -p ana_in.tbl.new >/dev/null" );

# open( STDOUT, ">", "ana_in2.tbl.new" ) || die;
# print "Cfg {\n";
# for my $bank ( 0, 1 ) {
#   print "  HBox { +-; \"Configuration Bank $bank\"; -+ };\n  \"\"";
#   print map( "  >\"$_\"", (0 .. 7) ), ";\n";
#   for my $row (0 .. 8 ) {
#     print "  \"$row:\"";
#     for my $col (0 .. 7 ) {
#       my $naddr = 0xC00 + $row*32 + $bank*16 + $col*2;
#       my $addr = sprintf( "%03X", $naddr );
#       my $chan = "AIC$addr";
#       print " ($chan,4)";
#       print "\n     " if $col == 3;
#     }
#     print ";\n";
#   }
# }
# print "}\n\n{\n",
#   "  HBox { |+; [Cfg]; |+ };\n  -;\n    >{ Time: (flttime,8) MFCtr: (MFCtr,5) }\n}\n";
# close STDOUT || warn "Error closing STDOUT\n";
# system( "phtable -p ana_in2.tbl.new >/dev/null" );
