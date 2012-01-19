#! /usr/bin/perl -w
# Responsible for generating:
#  idx64.idx idx64.tbl

my $srcdir = $ARGV[0];
my $n_idx = $ARGV[1] || die "Must specify IDX_N_CHANNELS\n";
my $odir = $srcdir || ".";
-d $srcdir || die "Source directory '$srcdir' not found\n";
open( my $idx, '>', "$srcdir/idx64.idx" ) ||
  die "Unable to write to '$srcdir/idx64.idx'\n";
open( my $tbl, '>', "$srcdir/idx64.tbl" ) ||
  die "Unable to write to '$srcdir/idx64.tbl'\n";

print $idx "Indexer Status 4 Hz IXStt\n";
print $tbl "Idx { HBox { +-; Title: Indexers; -+ };\n";

for my $idxr ( 0 .. $n_idx-1 ) {
  print $idx <<EOF

Channel $idxr Indexer Channel $idxr
  Position 4 Hz IX${idxr}Step
  Status 4 Hz IX${idxr}Stat Diagram IX${idxr}Dgrm KillStat IX${idxr}Kill
  Scan
  # Chop
  Scanstat IX${idxr}Scan
  Speed 5033 Hz
  Disable Zeroref
  # Swap Limits
  # Invert Step
  # Invert Direction
  # Invert Run
  Invert In Limit
  Invert Out Limit
  Invert Zero Ref
  Invert Kill A
  Invert Kill B
EOF
  ;

  print $tbl <<EOF
  "$idxr:" (IX${idxr}Step,5) (IX${idxr}Dgrm,5)
	  (IX${idxr}Scan,4) (IX${idxr}Kill,5);
EOF
  ;
}

close $idx;

print $tbl <<EOF
}

{ [Idx];
  -;
  >{ Time: (flttime,8) MFCtr: (MFCtr,5) }
}
EOF
  ;

close $tbl;
