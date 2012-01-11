#! /bin/bash
# Source code generator for generic DACS
# Takes a single argument, the experiment mnemonic
# dot-executes the configuration file $Experiment.txt
function nl_error {
  echo dacs_gen: $* >&2
  exit 1
}

[ -n "$1" ] || nl_error Must specify experiment
mnc=$1
[ -f "$mnc.txt" ] || nl_error "Cannot locate configuration file $mnc.txt"
. $mnc.txt
[ -n "$Experiment" ] || nl_error "Experiment not defined in $mnc.txt"
[ "$Experiment" = "$mnc" ] || nl_error "Experiment does not match configuration name"
[ -n "$HomeDir" ] || nl_error "HomeDir not defined in $mnc.txt"
[ -d "$HomeDir" ] || nl_error "HomeDir '$HomeDir' does not exist"
srcdir=$HomeDir/src/TM
[ -d $srcdir ] || mkdir -p $srcdir ||
  nl_error "Unable to create $srcdir"

echo 1.0 >$srcdir/VERSION
cp types.tmc $srcdir
cp cmdenbl.cmd $srcdir
cat <<EOF >$srcdir/Experiment.config
Experiment=$Experiment
HomeDir=$HomeDir
RUNFILE=interact
SUBBUSD=serusb
RunType=ask
# Extractions=
# Analysis=
EOF

tmcbase="types.tmc /usr/local/share/huarp/flttime.tmc"
cmdbase="/usr/local/share/huarp/root.cmd /usr/local/share/huarp/getcon.cmd cmdenbl.cmd"
tbl=""
col=""

function add_files {
  for f in $*; do
    [ -f $srcdir/$f ] || nl_error "File $srcdir/$f not found"
    case $f in
      *_col.tmc) col="$col $f";;
      *.tmc) tmcbase="$tmcbase $f";;
      *.cmd) cmdbase="$cmdbase $f";;
      *.tbl) tbl="$tbl $f";;
    esac
  done
}

# AI
cp AI.cmd AI.tbl $srcdir
./genai.pl $srcdir
add_files AI.tmc AI_col.tmc AI.cmd AI.tbl

# AO
./genao.pl $srcdir $N_AO_CHIPS
add_files AO.tmc AO.cmd AO.tbl

# PTRH
# QCLI
# Indexer
# Counters
# Voltage Monitor
# Syscon