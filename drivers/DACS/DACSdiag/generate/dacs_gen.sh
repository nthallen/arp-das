#! /bin/sh
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
. ./$mnc.txt
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
col="/usr/local/share/huarp/DACS_ID.tmc"
conv=""

function add_files_nochk {
  for f in $*; do
    case $f in
      *_col.*) col="$col $f";;
      *_conv.*) conv="$conv $f";;
      *.tmc) tmcbase="$tmcbase $f";;
      *.cmd) cmdbase="$cmdbase $f";;
      *.tbl) tbl="$tbl $f";;
    esac
  done
}

function add_files {
  for f in $*; do
    [ -f $srcdir/$f ] || nl_error "File $srcdir/$f not found"
  done
  add_files_nochk $*
}

# AI
cp AI.cmd AI.tbl $srcdir
./gen_ai.pl $srcdir
add_files AI.tmc AI_col.tmc AI.cmd AI.tbl

# AO
./gen_ao.pl $srcdir $N_AO_CHIPS
add_files AO.tmc AO_col.tmc AO.cmd AO.tbl

# PTRH
if [ -n "$N_PTRH" -a "$N_PTRH" != "0" ]; then
  cp ptrhm.cc ptrhm_col.cc ptrhm.h $srcdir
  ./gen_ptrh.pl $srcdir $N_PTRH
  add_files ptrhm_col.cc PTRH.tmc PTRH_col.tmc PTRH_conv.tmc PTRH.tbl
  tmcbase="$tmcbase ptrhm.cc"
fi

# QCLI
if [ -n "$N_QCLICTRL" -a "$N_QCLICTRL" != "0" ]; then
  cp qcli.cmd $srcdir
  ./gen_qcli.pl $srcdir $N_QCLICTRL
  wavefiles=''
  i=0
  while [ $i -lt $N_QCLICTRL ]; do
    perl -pe "s/\\@QCLI\\@/QCLI_$i/g" waves.qcli >$srcdir/waves$i.qcli
    # cp waves.qcli $srcdir/waves$i.qcli
    wavefiles="$wavefiles waves$i.cmd waves$i.tmc"
    let i=i+1
  done
  add_files qcli.cmd qclis.cmd qcli.tmc qcli_col.tmc qcli_conv.tmc qcli.tbl
  add_files_nochk $wavefiles
fi

# Indexer
# Counters
# Voltage Monitor
# Syscon

cat <<EOF >$srcdir/interact
# Startup script for DACS Diagnostic
  Launch memo memo -o \$Experiment.log
  memo=/dev/huarp/\$Experiment/memo
  Launch TM/DCo TMbfr
  echo "Running script interact" >\$memo
# Launch dccc   dccc -f \$TMBINDIR/dccc.dccc
  Launch DG/cmd ${Experiment}col
  Launch cmd/server ${Experiment}srvr
  Launch - lgr -N \`mlf_find LOG\`
# Launch - idx64 \`cat \$TMBINDIR/idx64.idx64\`
# Launch - ${Experiment}algo -v
EOF

chmod +x $srcdir/interact

cat <<EOF >$srcdir/$Experiment.doit
display ${Experiment}disp
client ${Experiment}clt
memo
EOF

# spec file
{
  for f in $tmcbase; do
    echo "tmcbase = $f"
  done
  for f in $cmdbase; do
    echo "cmdbase = $f"
  done
  echo
  echo "SCRIPT = interact"
  echo "TGTDIR = $HomeDir"
  echo
  echo "${mnc}srvr : -lsubbus"
  echo "${mnc}col : $col -lsubbus"
  echo "${mnc}disp : $conv $tbl"
  echo "doit : ${mnc}.doit"
  echo "%%"
  if [ -n "$N_QCLICTRL" -a "$N_QCLICTRL" != "0" ]; then
    i=0
    while [ $i -lt $N_QCLICTRL ]; do
      echo "waves$i.cmd waves$i.out waves$i.tmc waves$i.m : waves$i.qcli"
      echo "\tqclicomp -o waves$i.out -c waves$i.cmd -d waves$i.tmc \\"
      echo "\t  -v waves$i.log -m waves$i.m waves$i.qcli || \\"
      echo "\t  ( rm -f waves$i.out waves$i.cmd waves$i.tmc waves$i.log waves$i.m; false )"
      let i=i+1
    done
  fi
    
} >$srcdir/$mnc.spec
