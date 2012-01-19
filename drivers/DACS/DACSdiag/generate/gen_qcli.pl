#! /usr/bin/perl -w
# Responsible for generating:
#  qclis.cmd qcli.tmc qcli_col.tmc qcli_conv.tmc qcli.tbl

my $srcdir = $ARGV[0];
my $n_qcli = $ARGV[1] || die "Must specify N_QCLICTRL\n";
shift @ARGV;
shift @ARGV;
my $odir = $srcdir || ".";
-d $srcdir || die "Source directory '$srcdir' not found\n";
open( my $tmc, '>', "$srcdir/qcli.tmc" ) ||
  die "Unable to write to '$srcdir/qcli.tmc'\n";
open( my $cmd, '>', "$srcdir/qclis.cmd" ) ||
  die "Unable to write to '$srcdir/qclis.cmd'\n";
open( my $tbl, '>', "$srcdir/qcli.tbl" ) ||
  die "Unable to write to '$srcdir/qcli.tbl'\n";
open( my $conv, '>', "$srcdir/qcli_conv.tmc" ) ||
  die "Unable to write to '$srcdir/qcli_conv.tmc'\n";
open( my $col, '>', "$srcdir/qcli_col.tmc" ) ||
  die "Unable to write to '$srcdir/qcli_col.tmc'\n";

my @suffixes = (0 .. $n_qcli-1);
for my $i (0 .. $n_qcli-1) {
  if ( $i < @ARGV ) {
    $suffixes[$i] = $ARGV[$i];
  }
}

my @qcli = map "QCLI_$suffixes[$_]", (0 .. $n_qcli-1);
my @ssp = map "SSP_$suffixes[$_]", (0 .. $n_qcli-1);

print $tmc <<EOF
%{
  /* qcli.tmc generated by gen_qcli.pl */
  #include "qclid.h"
  #include "sspdrv.h"
  const char * SSP_Status_Text(const unsigned char stat) {
    switch (stat) {
      case SSP_STATUS_GONE:    return "   Gone";
      case SSP_STATUS_CONNECT: return "Connect";
      case SSP_STATUS_READY:   return "  Ready";
      case SSP_STATUS_ARMED:   return "  Armed";
      case SSP_STATUS_TRIG:    return "Trigger";
      default:                 return "*******";
    }
  }
%}

TM typedef unsigned char SSP_Status_t { text "%7d" SSP_Status_Text(); }
TM typedef unsigned short XLONG { text "%5d"; }

/* T_FPGA_t: MAX6628 output, resolution 1/16 degree
 *   reported as 128 bits per degree celcius */
TM typedef signed short T_FPGA_t { convert CELCIUS; text "%6.2lf"; }
Calibration ( T_FPGA_t, CELCIUS ) { 0, 0, 128, 1 }

/* T_HtSink_t: MAX6661 output. resolution 1/8 degree
  *  reported as 256 bits per degree */
TM typedef signed short T_HtSink_t { convert CELCIUS; text "%6.2lf"; }
Calibration ( T_HtSink_t, CELCIUS ) { 0, 0, 256, 1 }

EOF
;

for my $QN ( 0 .. $#qcli ) {
  my $QCLI = $qcli[$QN];
  print $tmc <<EOF
TM "Receive" ${QCLI} 1;

TM typedef unsigned short ${QCLI}_Wave_t { text "%10d" ${QCLI}_Wave_text(); }
TM 1 Hz ${QCLI}_Wave_t ${QCLI}_Wave;
TM 1 Hz UINT ${QCLI}_s;
TM 1 Hz UINT ${QCLI}_Stale;

Group ${QCLI}_grp ( ${QCLI}_Wave, ${QCLI}_s ) {
  ${QCLI}_Wave = ${QCLI}.qcli_wave;
  ${QCLI}_s = ${QCLI}.status;
  ${QCLI}_Stale = ${QCLI}_obj->stale();
  ${QCLI}_obj->synch();
}
EOF
  ;
  printf $tmc
    "TM 1 HZ XLONG ${QCLI}_CS; Address ${QCLI}_CS 0x%X;\n",
    0x1000 + $QN*0x10;

}

for my $SSP ( @ssp ) {
  print $tmc <<EOF
TM "Receive" ${SSP} 1;

TM 1 Hz L20 ${SSP}_Num;
TM 1 Hz L20 ${SSP}_SN;
TM 1 Hz L20 ${SSP}_TS;
TM 1 Hz XLONG ${SSP}_Flags;
TM 1 Hz SSP_Status_t ${SSP}_Status;
TM 1 Hz UINT ${SSP}_Stale;
TM 1 Hz T_FPGA_t ${SSP}_T_FPGA;
TM 1 Hz T_HtSink_t ${SSP}_T_HtSink;

Group ${SSP}_grp ( ${SSP}_Num, ${SSP}_SN, ${SSP}_TS, ${SSP}_Flags,
		 ${SSP}_T_FPGA, ${SSP}_T_HtSink ) {
  ${SSP}_Num = ${SSP}.index;
  ${SSP}_SN = ${SSP}.ScanNum;
  ${SSP}_TS = ${SSP}.Total_Skip;
  ${SSP}_Flags = ${SSP}.Flags;
  ${SSP}_Status = ${SSP}.Status;
  ${SSP}_T_FPGA = ${SSP}.T_FPGA & 0xFFF8;
  ${SSP}_T_HtSink = ${SSP}.T_HtSink & 0xFFE0;
  ${SSP}_Stale = ${SSP}_obj->stale();
  ${SSP}_obj->synch();
}
EOF
  ;
}

# qclis.cmd

print $cmd
  map( "%INTERFACE \<$_>\n", @ssp, @qcli );

print $cmd <<EOF

%{

#ifdef SERVER
  #include "hsatod.h"
  #define QCLI_ICOS \\
      (HSAD_OPT_A|HSAD_OPT_B|HSAD_OPT_C|HSAD_TRIG_3|HSAD_TRIG_RISING)
EOF
  ;

print $cmd
  map( "  #define ${_}_ICOS QCLI_ICOS\n", @qcli ),
  map( "  hsatod_setup_t ${_}_setup;\n", @ssp );

print $cmd <<EOF

  static struct sspqcli_s {
    hsatod_setup_t *setup;
    cmdif_rd *if_ssp;
  } sspqcli_bd[$n_qcli] = {
EOF
  ;

print $cmd
  map( "    { &$ssp[$_]_setup, &if_$ssp[$_] },\n", (0 .. $#qcli)),
  <<EOF
  };

#endif

%}
EOF
  ;

print $cmd
  "# &SSP returns an index into sspqcli_bd[]\n",
  "&SSP <int>\n",
  map( "  : $ssp[$_] { \$0 = $_; }\n", (0 .. $#ssp) ),
  "  ;\n",
  "# &QCLI returns an inteface\n",
  "&QCLI <cmdif_rd *>\n",
  map( "  : $_ { \$0 = &if_$_; }\n", @qcli),
  "  ;\n",
  "&command\n";

for my $QCLI ( 0 .. $#qcli ) {
  print $cmd <<EOF
  : Select $qcli[$QCLI] Waveform &$qcli[$QCLI]_Wave * {
      *sspqcli_bd[$QCLI].setup = $qcli[$QCLI]_Waves[\$4];
      if_$ssp[$QCLI].Turf( "SW:%d\\n", \$4 );
    }
EOF
  ;
}

print $cmd "  ;\n";

print $tbl "# qcli.tbl generated by gen_qcli.pl\n";
for my $QN ( 0 .. $#qcli ) {
  my $QCLI = $qcli[$QN];
  my $SSP = $ssp[$QN];

  print $tbl <<EOF

$QCLI {
  HBox { +-; Title: "$QCLI"; -+ };
  Wave: HBox { (${QCLI}_Wave,10); HGlue 0+1 };
  Mode: { (${QCLI}_mode,7) (${QCLI}_laser,3) 
          Stale: (${QCLI}_Stale,5) };
  SSP { (${SSP}_Num,8) (${SSP}_Status,7); }
}

${SSP} {
  HBox { +-; Title: "${SSP}"; -+ };
  HBox {
    { File: (${SSP}_Num,8);
      Scan: (${SSP}_SN,8);
      Skip: (${SSP}_TS,8);
      Stale: (${SSP}_Stale,5);
    }; +|; {
      ""  >"1|2|3 ";
      AD: (${SSP}_AD,5);
      PA: (${SSP}_PA,5);
      CA: (${SSP}_CA,5);
    }
  }
}

${SSP}_T {
  { HBox { +-; Title: "${SSP} Temps"; -+ };
    { FPGA: (${SSP}_T_FPGA,6) C HtSink: (${SSP}_T_HtSink,6) C }
  }
}

${QCLI}_S {
  HBox { +-; Title: "${QCLI} Status"; -+ };
  HBox {
    {
      Rdy:     (${QCLI}_ready,3);
      Busy:    (${QCLI}_busy,3);
      Sel:     (${QCLI}_waveerr,4);
      Flsh:    (${QCLI}_flash,2);
      Cksm:    (${QCLI}_cksum,1);
    }; |; {
      "DOT:"   (${QCLI}_dot,4);
      "LOT:"   (${QCLI}_lot,4);
      "LOC:"   (${QCLI}_loc,4);
      "COR:"   (${QCLI}_cordte,4);
      "CErr:"  (${QCLI}_cmderr,4);
    }; |; {
      Com:  (${QCLI}_present,4);
      Act:  (${QCLI}_rw,2);
      FIFO: (${QCLI}_fifodep,3);
      Err:  (${QCLI}_err,2);
    }
  }
}

${QCLI}_Col {
  [${QCLI}];
  [${SSP}];
  [${SSP}_T];
  [${QCLI}_S];
}
EOF
;
}

print $tbl
  "{ HBox { |+;\n ",
  map( " [${_}_Col]; |+;", @qcli ),
  " };\n",
  "  -;\n",
  "  >{ Time: (flttime,8) MFCtr: (MFCtr,5) }\n",
  "}\n";

print $col
  "%{\n",
  map( "  ssp_data_t $_;\n", @ssp ),
  map( "  qcli_data_t $_;\n", @qcli ),
  "%}\n";

print $conv <<EOF
%{
  const char *ok_fail_text[] = { "  ok", "FAIL" };
  const char *no_yes_text[]  = { " no", "yes" };
  const char *rw_text[] = { "--", "-W", "R-", "RW" };
  const char *qclimode_text[] = {
	"   idle",
	"program",
	"psector",
	"    run",
	" select",
	"-------",
	"-------",
	"======="
  };
  const char *ovf_text[] = {
    " | | ",
    "*| | ",
    " |*| ",
    "*|*| ",
    " | |*",
    "*| |*",
    " |*|*",
    "*|*|*" };
%}
TM typedef int ok_fail_t { text "%4d" ok_fail_text[]; }
TM typedef int no_yes_t { text "%3d" no_yes_text[]; }
TM typedef int onebit_t { text "%1d"; }
TM typedef int twobits_t { text "%02b"; }
TM typedef int qclimode_t { text "%7d" qclimode_text[]; }
TM typedef int SSP_OVF { text "%5d" ovf_text[]; }
TM typedef int rw_t { text "%2d" rw_text[]; }
TM typedef unsigned char fifodep_t { text "%3u"; }
TM typedef unsigned char qclierr_t { text "%02x"; }
EOF
;
for my $QCLI ( @qcli ) {
  print $conv <<EOF

no_yes_t ${QCLI}_busy; invalidate ${QCLI}_busy;
  { ${QCLI}_busy = (${QCLI}_s & 0x8000) ? 1 : 0;
    validate ${QCLI}_busy; }
onebit_t ${QCLI}_cksum; invalidate ${QCLI}_cksum;
  { ${QCLI}_cksum = (${QCLI}_s & 0x4000) ? 1 : 0;
    validate ${QCLI}_cksum; }
ok_fail_t ${QCLI}_cmderr; invalidate ${QCLI}_cmderr;
  { ${QCLI}_cmderr = (${QCLI}_s & 0x2000) ? 1 : 0;
    validate ${QCLI}_cmderr; }
off_on_t ${QCLI}_laser; invalidate ${QCLI}_laser;
  { ${QCLI}_laser = (${QCLI}_s & 0x1000) ? 0 : 1;
    validate ${QCLI}_laser; }
ok_fail_t ${QCLI}_cordte; invalidate ${QCLI}_cordte;
  { ${QCLI}_cordte = (${QCLI}_s & 0x0800) ? 1 : 0;
    validate ${QCLI}_cordte; }
no_yes_t ${QCLI}_ready; invalidate ${QCLI}_ready;
  { ${QCLI}_ready = (${QCLI}_s & 0x0200) ? 1 : 0;
    validate ${QCLI}_ready; }
ok_fail_t ${QCLI}_waveerr; invalidate ${QCLI}_waveerr;
  { ${QCLI}_waveerr = (${QCLI}_s & 0x0100) ? 1 : 0;
    validate ${QCLI}_waveerr; }
twobits_t ${QCLI}_flash; invalidate ${QCLI}_flash;
  { ${QCLI}_flash = (${QCLI}_s & 0x00C0) >> 6;
    validate ${QCLI}_flash; }
ok_fail_t ${QCLI}_dot; invalidate ${QCLI}_dot;
  { ${QCLI}_dot = (${QCLI}_s & 0x0020) ? 1 : 0;
    validate ${QCLI}_dot; }
ok_fail_t ${QCLI}_lot; invalidate ${QCLI}_lot;
  { ${QCLI}_lot = (${QCLI}_s & 0x0010) ? 1 : 0;
    validate ${QCLI}_lot; }
ok_fail_t ${QCLI}_loc; invalidate ${QCLI}_loc;
  { ${QCLI}_loc = (${QCLI}_s & 0x0008) ? 1 : 0;
    validate ${QCLI}_loc; }
qclimode_t ${QCLI}_mode; invalidate ${QCLI}_mode;
  { ${QCLI}_mode = ${QCLI}_s & 0x7; validate ${QCLI}_mode; }
rw_t ${QCLI}_rw; invalidate ${QCLI}_rw;
  { ${QCLI}_rw = ((${QCLI}_CS & 0x4000) ? 2 : 0) +
                ((${QCLI}_CS & 0x400) ? 1 : 0);
    validate ${QCLI}_rw;
  }
ok_fail_t ${QCLI}_present; invalidate ${QCLI}_present;
  { ${QCLI}_present = (${QCLI}_CS & 0x800) ? 0 : 1;
    validate ${QCLI}_present;
  }
fifodep_t ${QCLI}_fifodep; invalidate ${QCLI}_fifodep;
  { ${QCLI}_fifodep = ${QCLI}_CS & 0xFF;
    validate ${QCLI}_fifodep;
  }
qclierr_t ${QCLI}_err; invalidate ${QCLI}_err;
  { ${QCLI}_err = ((${QCLI}_CS >> 8) & 0xBB) ^ 0x8;
    validate ${QCLI}_err;
  }
EOF
;
}

for my $SSP ( @ssp ) {
  print $conv <<EOF

SSP_OVF ${SSP}_CA; Invalidate ${SSP}_CA;
SSP_OVF ${SSP}_PA; Invalidate ${SSP}_PA;
SSP_OVF ${SSP}_AD; Invalidate ${SSP}_AD;
{ ${SSP}_CA = SSP_CAOVF(${SSP}_Flags); Validate ${SSP}_CA; }
{ ${SSP}_PA = SSP_PAOVF(${SSP}_Flags); Validate ${SSP}_PA; }
{ ${SSP}_AD = SSP_ADOOR(${SSP}_Flags); Validate ${SSP}_AD; }
EOF
  ;
}

close $tmc;
close $cmd;
close $tbl;
close $conv;
close $col;