#! /usr/bin/perl -w
# Responsible for generating AO.tmc AO_col.tmc and AO.cmd

my $srcdir = $ARGV[0];
my $n_ao_chips = $ARGV[1] || 2;
my $odir = $srcdir || ".";
-d $srcdir || die "Source directory '$srcdir' not found\n";
open( my $tmc, '>', "$srcdir/AO.tmc" ) ||
  die "Unable to write to '$srcdir/AO.tmc'\n";
open( my $col, '>', "$srcdir/AO_col.tmc" ) ||
  die "Unable to write to '$srcdir/AO_col.tmc'\n";
open( my $tbl, '>', "$srcdir/AO.tbl" ) ||
  die "Unable to write to '$srcdir/AO.tbl'\n";
open( my $cmd, '>', "$srcdir/AO.cmd" ) ||
  die "Unable to write to '$srcdir/AO.cmd'\n";

print $tmc
  "%{\n  /* AO.tmc generated by gen_ao.pl */\n",
  "  #include \"subbus.h\"\n",
  "  extern unsigned short AO_buf[8];\n",
  "%}\n";

print $col
  "%{\n",
  "  /* AO_col.tmc generated by gen_ao.pl */\n",
  "  unsigned short AO_buf[8];\n",
  "  void ao_init(void) {\n";

print $cmd <<'EOF'
&command
  : Set &ao_chan %lf (Enter voltage 0-10) * {
      double N = $3 * 6553.6;
      unsigned short bits;
      if (N > 65535) N = 65535;
      if (N < 0) N = 0.;
      bits = (unsigned short) N;
      sbwr( $2, bits );
    }
  ;
&ao_chan <unsigned short>
EOF
;

for my $chip (0 .. $n_ao_chips-1) {
  my @group;
  print $tbl "AO${chip} {\n";
  for my $col (0 .. 7 ) {
    my $naddr = 0x400 + $chip*16 + $col*2;
    my $addr = sprintf( "%03X", $naddr );
    my $chan = "AO$addr";
    print $tmc "TM 1 Hz AO16 $chan; Address $chan 0x$addr;\n";
    push @group, $chan;
    print $tbl "  \"$addr:\" ($chan,6) V;\n";
    print $cmd "  : $chan { \$0 = 0x$addr; }\n";
  }
  print $tmc "\n%{\n  subbus_mread_req *AO${chip}_req;\n%}\n";
  printf $col
    "    AO${chip}_req = pack_mread_request( 8, \"%X:2:%X\" );\n",
    0x400 + $chip*16, 0x400 + $chip*16 + 7*2;
  print $tmc "\nGroup AO${chip} ( ",
    join( ", ", @group ), " ) {\n",
    "  mread_subbus( AO${chip}_req, AO_buf );\n",
    map( "  $group[$_] = AO_buf[$_];\n", (0 .. 7) ),
    "}\n\n";
  print $tbl "}\n";
}


print $col "  }\n%}\nTM INITFUNC ao_init();\n";
print $tbl
  "{\n",
  "  HBox { +-; \"Analog Outputs\"; -+ };\n",
  "  HBox {|+;",
  map( " [AO$_]; |+;", (0 .. $n_ao_chips-1) ),
  " };\n",
  "  -;\n",
  "  >{ Time: (flttime,8) MFCtr: (MFCtr,5) }\n",
  "}\n";
print $cmd "  ;\n";
  
close $tmc;
close $col;
close $tbl;
close $cmd;