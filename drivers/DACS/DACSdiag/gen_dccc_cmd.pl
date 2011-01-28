#! /usr/bin/perl -w

print <<EOF;
%INTERFACE <dccc>
&command
  : &dccc_cmd * { cis_turf(if_dccc, "D%d\\n", \$1); }
  ;
&dccc_cmd <int>
EOF

use integer;
for my $i (0 .. 63) {
  my $cmd = $i/2;
  my $status = ($i & 1) ? "Off" : "On";
  print "  : Command $cmd $status { \$0 = $i; }\n";
}

print "  ;\n";

