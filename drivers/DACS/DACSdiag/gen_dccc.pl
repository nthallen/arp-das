#! /usr/bin/perl -w

# Generate initial dccc.dccc file for 32 commands
# We will have *no* configuration commands, since
# the configuration will be hard-coded into the DACS

print <<EOF;
0

8
EOF

use integer;
for my $port (0 .. 7) {
  my $conn = $port/6;
  my $addr = 0x800 + $conn*0x20 + ($port&1 ? 0x10 : 0x8) +
      (($port - $conn*6)/2)*2;
  printf "  0x%04X, 0\n", $addr;
}

print "\n64\n";
for my $cmdnum (0 .. 63) {
  my $port = $cmdnum/8;
  my $bit = $cmdnum % 16;
  my $cmd = $cmdnum/2;
  my $text = "Cmd $cmd " . ( $bit & 1 ? "Off" : "On" );
  printf "  STRB,   %2d, 0x%04X ; %d: %s\n",
    $port, 1<<$bit, $cmdnum, $text;
}
