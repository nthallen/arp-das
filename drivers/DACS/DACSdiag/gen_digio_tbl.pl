#! /usr/bin/perl -w

open STDOUT, ">", "digio.tbl.new" || die;
for my $bank ( 0 .. 2 ) {
  print "Cmd_$bank {\n",
    '  ""  DS  >S<  >I<;', "\n";
  for my $col ( 0 .. 7 ) {
    my $cmd = $bank*8+$col;
    my $addr = 0xD10 + 0x20*$bank +
      (($bank == 0) ? $col : (7-$col))*2;
    my $Ichan = sprintf("AI%03X", $addr);
    print "  \"$cmd:\" (Cmd${cmd}DS,3) (Cmd${cmd}S,3) ($Ichan,7);\n";
  }
  print "}\n\n";
}
for my $bank ( 3 ) {
  print "Cmd_$bank {\n",
    '  ""  DS;', "\n";
  for my $col ( 0 .. 7 ) {
    my $cmd = $bank*8+$col;
    print "  \"$cmd:\" (Cmd${cmd}DS,3);\n";
  }
  print "}\n\n";
}

print <<EOF;
{ HBox { +-; Commands; -+ };
  HBox { +|; [Cmd_0]; +|; [Cmd_1]; +|; [Cmd_2]; +|; [Cmd_3]; +| };
  -;
  >{ Time: (flttime,8) MFCtr: (MFCtr,5) }
}
EOF

close STDOUT || warn "Error closing digio.tbl.new\n";
system("phtable -p digio.tbl.new >/dev/null");

