#! /usr/bin/perl -w
print "AD {\n";
for my $bank ( 0, 1 ) {
  print "  HBox { +-; \"Bank $bank\"; -+ };\n  \"\"";
  print map( "  >\"$_\"", (0 .. 7) ), ";\n";
  for my $row (0 .. 7 ) {
    print "  \"$row:\"";
    for my $col (0 .. 7 ) {
      my $addr = sprintf( "C%02X", $row*32 + $bank*16 + $col*2 );
      my $chan = "AI$addr";
      # my $caddr = sprintf( "C%02X", $row*32 + $bank*16 + $col*2 + 1 );
      # my $cchan = "AIC$addr";
      print " ($chan,6)";
    }
    print ";\n";
  }
}
for my $bank ( 0, 1 ) {
  print "  >HBox { -; \"Configuration Bank $bank\"; - }<;\n  \"\"";
  print map( "  >\"$_\"", (0 .. 7) ), ";\n";
  for my $row (0 .. 7 ) {
    print "  \"$row:\"";
    for my $col (0 .. 7 ) {
      my $addr = sprintf( "C%02X", $row*32 + $bank*16 + $col*2 );
      my $chan = "AIC$addr";
      print " ($chan,4)";
    }
    print ";\n";
  }
}
print "}\n\n{\n",
  "  HBox { |+; [AD]; |+ };\n  -;\n    >{ Time: (flttime,8) MFCtr: (MFCtr,5) }\n}\n";
