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
SUBBUSD=serusb
RunType=ask
# Extractions=
# Analysis=
EOF

if [ -n "$FlightNode" ]; then
  cat <<EOF >>$srcdir/Experiment.config
FlightNode=$FlightNode
SCRIPT_OVERRIDE=/net/$FlightNode$HomeDir/script
RUNFILE=runfile
EOF
else
  echo "RUNFILE=interact" >>$srcdir/Experiment.config
fi

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
    case $f in
      /*) fname=$f;;
      *) fname=$srcdir/$f;;
    esac
    [ -f $fname ] || nl_error "File $fname not found"
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
QCLI_SCRIPT=''
if [ -n "$N_QCLICTRL" -a "$N_QCLICTRL" != "0" ]; then
  cp qcli.cmd $srcdir
  ./gen_qcli.pl $srcdir $N_QCLICTRL
  wavefiles=''
  i=0
  while [ $i -lt $N_QCLICTRL ]; do
    perl -pe "s/\\@QCLI\\@/QCLI_$i/g" waves.qcli >$srcdir/waves$i.qcli
    # cp waves.qcli $srcdir/waves$i.qcli
    wavefiles="$wavefiles waves$i.cmd waves$i.tmc"
    QCLI_SCRIPT="$QCLI_SCRIPT waves$i.out"
    let i=i+1
  done
  add_files qcli.cmd qclis.cmd qcli.tmc qcli_col.tmc qcli_conv.tmc qcli.tbl
  add_files_nochk $wavefiles
fi

# Digio
./gen_digio.pl $srcdir CP_NCMDS=$CP_NCMDS CP_DS=$CP_DS PB_S=$PB_S
add_files digio.tmc digio_conv.tmc digio.cmd digio.tbl
# digio.dccc will be handled manually below in .spec and interact

# Indexer
if [ -n "$IDX_N_CHANNELS" -a "$IDX_N_CHANNELS" != "0" ]; then
  ./gen_idx.pl $srcdir $IDX_N_CHANNELS
  ( cd $srcdir; idx64cfg idx64.idx idx64 )
  add_files /usr/local/share/huarp/idx64.cmd idx64drv.cmd
  add_files idx64.tmc idx64col.tmc idx64.tbl
  conv="$conv idx64flag.tmc"
fi

# Counters
if [ -n "$CTR_UG_N_BDS" -a "$CTR_UG_N_BDS" != "0" ]; then
  ./gen_ctr.pl $srcdir $CTR_UG_N_BDS
  add_files ctr.tmc ctr_conv.tmc ctr_col.tmc ctr.tbl
fi

# Power Monitor
if [ -n "$N_PWRMON" -a "$N_PWRMON" != "0" ]; then
  ./gen_pm.pl $srcdir $N_PWRMON
  add_files pwrmon.tmc pwrmon_conv.tmc pwrmon.tbl
fi

# Syscon
{
  cat <<EOF
State Init {
      > Telemetry Start
   +1 > CmdEnbl On
}
EOF
} >$srcdir/$mnc.tma

have_idx='#'
idx_script=''
if [ -n "$IDX_N_CHANNELS" -a "$IDX_N_CHANNELS" != "0" ]; then
  have_idx=' '
  idx_script=' idx64.idx64'
fi

{
  cat <<EOF
# Startup script for DACS Diagnostic
  Launch memo memo -o \$Experiment.log
  memo=/dev/huarp/\$Experiment/memo
  Launch TM/DCo TMbfr
  echo "Running script interact" >\$memo
EOF

  if [ -n "$N_QCLICTRL" -a "$N_QCLICTRL" != "0" ]; then
    echo "\n# Verify and/or Program QCLIs"
    i=0
    while [ $i -lt $N_QCLICTRL ]; do
      echo "  qclidprog -h QCLI_$i -d$i -cwc \$TMBINDIR/waves$i.out"
      let i=i+1
    done
    echo
  fi

  cat <<EOF
  Launch dccc   dccc -f \$TMBINDIR/digio.dccc
  Launch DG/cmd ${Experiment}col
  Launch cmd/server ${Experiment}srvr
  Launch - lgr -N \`mlf_find LOG\`
$have_idx Launch - idx64 \`cat \$TMBINDIR/idx64.idx64\`
EOF

  if [ -n "$N_QCLICTRL" -a "$N_QCLICTRL" != "0" ]; then
    i=0
    while [ $i -lt $N_QCLICTRL ]; do
      echo "  Launch - sspdrv -b$i -hSSP_$i -N \`mlf_find SSP_$i\`"
      echo "  Launch - qclidacsd -h QCLI_$i -d $i"
      let i=i+1
    done
  fi

  echo "  Launch - ${Experiment}algo -v"
} >$srcdir/interact

chmod +x $srcdir/interact

cat <<EOF >$srcdir/$Experiment.doit
display ${Experiment}disp
client ${Experiment}clt
batchfile interact
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
  echo "SCRIPT = interact digio.dccc$idx_script"
  [ -n "$QCLI_SCRIPT" ] && echo "SCRIPT =$QCLI_SCRIPT"
  echo "TGTDIR = \$(TGTNODE)$HomeDir"
  echo
  echo "${mnc}srvr : -lsubbus"
  echo "${mnc}col : $col -lsubbus"
  echo "${mnc}disp : $conv $tbl"
  echo "${mnc}algo : $mnc.tma"
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
( cd $srcdir; appgen; )
