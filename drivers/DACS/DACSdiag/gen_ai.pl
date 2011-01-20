#! /usr/bin/perl -w
for my $col (0 .. 7 ) {
  for my $bank ( 0, 1 ) {
    my @group;
    for my $row (0 .. 7 ) {
      my $addr = sprintf( "C%02X", $row*32 + $bank*16 + $col*2 );
      my $chan = "AI$addr";
      my $caddr = sprintf( "C%02X", $row*32 + $bank*16 + $col*2 + 1 );
      my $cchan = "AIC$addr";
      print "TM 1/2 Hz AI16 $chan; Address $chan 0x$addr;\n",
	    "TM 1/2 Hz AIC $cchan; Address $cchan 0x$caddr;\n";
      push @group, $chan, $cchan;
    }
    print "\nGroup Col${col}Bank$bank ( ",
      join( ", ", @group ), " ) {\n",
      map( "  $_ = sbrwa($_.address);\n", @group ),
      "}\n\n";
  }
}
