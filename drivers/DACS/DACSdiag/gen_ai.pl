#! /usr/bin/perl -w
for my $col (0 .. 7 ) {
  for my $bank ( 0, 1 ) {
    my @group;
    for my $row (0 .. 7 ) {
      my $naddr = 0xC00 + $row*32 + $bank*16 + $col*2;
      my $addr = sprintf( "%03X", $naddr );
      my $chan = "AI$addr";
      print "TM 1/2 Hz AI16 $chan; Address $chan 0x$addr;\n";
      push @group, $chan;
      # my $caddr = sprintf( "%03X", $naddr+1 );
      # my $cchan = "AIC$addr";
      # print "TM 1/2 Hz AIC $cchan; Address $cchan 0x$caddr;\n";
      # push @group, $cchan;
    }
    print "\nGroup Col${col}Bank$bank ( ",
      join( ", ", @group ), " ) {\n",
      map( "  $_ = sbrwa($_.address);\n", @group ),
      "}\n\n";
  }
}

# Three rows of muxed channels
for my $row ( 8 .. 10 ) {
  my $bank = 1;
  my @group;
  for my $col ( 0 .. 7 ) {
    my $naddr = 0xC00 + $row*32 + $bank*16 + $col*2;
    my $addr = sprintf( "%03X", $naddr );
    my $chan = "AI$addr";
    print "TM 1/2 Hz AI16 $chan; Address $chan 0x$addr;\n";
    push @group, $chan;
    # my $caddr = sprintf( "%03X", $naddr+1 );
    # my $cchan = "AIC$addr";
    # print "TM 1/2 Hz AIC $cchan; Address $cchan 0x$caddr;\n";
    # push @group, $cchan;
  }
  print "\nGroup Mux${row} ( ",
    join( ", ", @group ), " ) {\n",
    map( "  $_ = sbrwa($_.address);\n", @group ),
    "}\n\n";
}
