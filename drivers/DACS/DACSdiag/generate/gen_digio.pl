#! /usr/bin/perl -w
# Responsible for generating:
#  digio.cmd digio.tmc digio_conv.tmc digio.tbl digio.dccc
#
# ./gen_digio $srcdir [KW=# ...]

die "Must specify srcdir\n" unless @ARGV;
my $srcdir = shift @ARGV;

# These defaults are probably wrong for most boards:
my $CP_NCMDS = 24;
my $CP_DS = 48;
my $PB_S = 72;
my $N_CONN = 3;

my %collected;

while ( @ARGV ) {
  my $arg = shift @ARGV;
  $arg =~ m/^(\w+)=(\d+)$/ ||
    die "Invalid argument: '$arg'\n";
  my $kw = $1;
  my $val = $2;
  if ( $kw eq 'CP_NCMDS' ) {
    $CP_NCMDS = $val;
  } elsif ( $kw eq 'CP_DS' ) {
    $CP_DS = $val;
  } elsif ( $kw eq 'PB_S' ) {
    $PB_S = $val;
  } elsif ( $kw eq 'N_CONN' ) {
    $N_CONN = $val;
  } else {
    die "Unrecognized keyword: '$kw'\n";
  }
}

die "CP_DS must be divisible by 8\n" if $CP_DS % 8;
die "CP_DS must be >= CP_NCMDS*2\n" unless $CP_DS >= $CP_NCMDS*2;
die "PB_S must be >= CP_DS+24\n" unless $PB_S >= $CP_DS+24;

-d $srcdir || die "Source directory '$srcdir' not found\n";
open( my $tmc, '>', "$srcdir/digio.tmc" ) ||
  die "Unable to write to '$srcdir/digio.tmc'\n";
open( my $tbl, '>', "$srcdir/digio.tbl" ) ||
  die "Unable to write to '$srcdir/digio.tbl'\n";
open( my $conv, '>', "$srcdir/digio_conv.tmc" ) ||
  die "Unable to write to '$srcdir/digio_conv.tmc'\n";
open( my $cmd, '>', "$srcdir/digio.cmd" ) ||
  die "Unable to write to '$srcdir/digio.cmd'\n";
open( my $dccc, '>', "$srcdir/digio.dccc" ) ||
  die "Unable to write to '$srcdir/digio.dccc'\n";

my $max_port;
my $max_conn;
{ use integer;
  $max_port = ($PB_S+23)/8;
  $max_conn = $max_port/6;
}
my $n_dccc_cmd = ($max_port+1)*8;
my @offset = ( 8, 17, 10, 19, 12, 21 );

print $cmd
  "%INTERFACE <dccc:dccc>\n",
  "&off_on <int>\n",
  "  : Off { \$0 = 1; }\n",
  "  : On { \$0 = 0; } \n",
  "  ;\n",
  "&command\n",
  "  : &dccc_cmd &off_on * { if_dccc.Turf(\"D%d\\n\", \$1*2 + \$2); }\n",
  "  ;\n",
  "&dccc_cmd <int>\n",
  map( "  : Command $_ { \$0 = $_; }\n", (0 .. $CP_NCMDS-1) ),
  "  ;\n";
close $cmd;

# dccc
print $dccc "0\n\n";

{ use integer;
  my $max_cmd_port = (2*$CP_NCMDS-1)/8;
  for my $connport ( 0 .. $max_cmd_port ) {
    my $conn = $connport/6;
    my $port = $connport%6;
    my $addr = 0x800 + $conn * 0x20 + $offset[$port];
    printf $dccc "  0x%04X, 0\n", $addr;
  }

  print $dccc "\n", 2*$CP_NCMDS, "\n";
  for my $cmd ( 0 .. $CP_NCMDS-1 ) {
    for my $state ( 0, 1 ) {
      my $dccccmd = 2*$cmd + $state;
      my $connport = $dccccmd/8;
      my $bit = $dccccmd % 16;
      printf $dccc "  STRB, %2d, 0x%04X ; %2d: Cmd %d %s\n",
	$connport, 1<<$bit, $dccccmd, $cmd,
	$state ? "Off" : "On";
    }
  }
}
close $dccc;

print $tmc
  "TM typedef unsigned char DStat {\n",
  "  text \"%02X\"; collect x = sbrba(x.address); }\n";
gen_tmc( "DS", $CP_DS, $CP_NCMDS );
gen_tmc( "S", $PB_S, 24 );

close $tmc;
close $conv;

# Generate columns of 8 commands
{ use integer;
  my $max_col = ($CP_NCMDS-1)/8;
  for my $col ( 0 .. $max_col ) {
    print $tbl "Cmd_$col {\n  ", '""  DS',
      $col < 3 ? '  >S<  >I<' : '',
      ";\n";
    for my $n ( 0 .. 7 ) {
      my $cmd = $col*8 + $n;
      if ( $cmd < $CP_NCMDS ) {
	if ( $col < 3 ) {
	  my $order = $col == 0 ? $n : (7-$n);
	  my $aiaddr = 0x10 + 0x20 * $col + 2*$order;
	  print $tbl "  \"$cmd:\" (Cmd_${cmd}_DS,3) (Cmd_${cmd}_S,3) ";
	  printf $tbl "(AID%02X,5);\n", $aiaddr;
	} else {
	  print $tbl "  \"$cmd:\" (Cmd_${cmd}_DS,3);\n";
	}
      }
    }
    print $tbl "}\n";
  }
  print $tbl
    "{ HBox { +-; Commands; -+ };\n",
    "  HBox { +|",
    map( "; [Cmd_$_]; +|", (0 .. $max_col ) ),
    " };\n",
    "  -;\n",
    "  >{ Time: (flttime,8) MFCtr: (MFCtr,5) }\n",
    "}\n";
}
close $tbl;

sub gen_tmc {
  use integer;
  my ( $suff, $dig_IO, $ns ) = @_;
  for my $cmd ( 0 .. $ns-1 ) {
    my $pin = $cmd + $dig_IO;
    my $connport = $pin/8;
    my $conn = $connport/6;
    my $port = $connport%6;
    my $addr = 0x800 + 0x20 * $conn + $offset[$port];
    my $xaddr = sprintf "%3X", $addr;
    my $pvar = "DS$xaddr";
    my $bit = $pin%8;
    my $var = "Cmd_${cmd}_$suff";
    unless ( $collected{$pvar} ) {
      $collected{$pvar} = 1;
      print $tmc "TM 1 Hz DStat $pvar; Address $pvar 0x$xaddr;\n";
    }
    print $conv
      "off_on_t $var; invalidate $var;\n",
      "{ $var = ($pvar >> $bit) & 1; Validate $var; }\n";
  }
}