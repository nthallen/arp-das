#! /bin/sh
# Run tests
function nl_error {
  echo "harness: $*" >&2
  exit 1
}

function report {
  cmd=$1
  status=$2
  reason=$3
  echo "$status: '$cmd' $reason"
}

function compare {
  type=$1
  src=$2
  compfile=$3
  # echo "# type=$1 src=$2 compfile=$3"
  if [ "$src" = "Makefile" ]; then
    grep -v "^# Makefile Generated" Makefile >Makefile.tmp
    mv -f Makefile.tmp Makefile
  fi
  if [ -n "$compfile" -a "$compfile" != '-' ]; then
    compfile=$cmpdir$compfile
  else
    compfile=/dev/null
  fi
  # echo "# compfile=$compfile"
  if [ -r "$compfile" ]; then
    if cmp -s $src $compfile; then
      rm $src
      # echo "# Compare OK"
    else
      status=FAIL
      if [ $compfile = '/dev/null' ]; then
        # echo "# Compare fail, looking for good replacement name"
        i=1
        savefile=$cmpdir/${type}_$i
        while [ -f $savefile.err ]; do
          let i=i+1
          savefile=$cmpdir/${type}_$i
        done
      else
        savefile=$compfile
      fi
      report "$mod" FAIL "$type output changed: Output saved to $savefile.err"
      mv $src $savefile.err
      diff $compfile $savefile.err
    fi
  else
    report "$mod" WARN "$type Output file created, not compared"
    mv $src $compfile
  fi
}

function harnessit {
  cmd=$1
  mod=$2
  ok=$3
  compfile=$4
  errfile=$5
  shift; shift; shift; shift;
  [ $# -gt 0 ] && shift;
  # echo " cmd='$cmd'"
  # echo " mod='$mod'"
  # echo " ok ='$ok'"
  # echo " compfile='$compfile'"
  # echo " errfile='$errfile'"
  # echo " xtra='$*'"
  status=OK
  okstat=OK
  cmpdir='output/'
  [ -n "$compfile" ] ||
    nl_error "Must specify compfile for command '$mod'"
  $cmd $mod >output.txt 2>error.txt || okstat=FAIL
  [ "$ok" = "$okstat" ] || {
    status=FAIL
    report "$mod" FAIL "expected $ok, saw $okstat"
  }
  compare stdout output.txt $compfile
  compare stderr error.txt $errfile
  while [ -n "$1" ]; do
    # echo "running compare on $1 $2"
    compare generated $1 $2
    shift; shift;
  done
  [ $status = OK ] &&
    report "$mod" OK
}

function harness {
  mod=$1
  shift
  harnessit ../../appgen_module "$mod" $*
}
function harness_ag {
  harnessit ../../appgen $*
}

# Test module help
harness moddef1 OK moddef1_help.stdtxt
# Test default mode
harness 'moddef1 X' OK moddef1_X.stdtxt
# Test disable mode
harness 'moddef1 X mode=disable' OK moddef1_d.stdtxt
# Test nonexistant mode
harness 'moddef1 X mode=noexistant' OK moddef1_n.stdtxt
# moddef2: Variable substitutions
harness 'moddef2' OK moddef2_help.stdtxt
harness 'moddef2 X' OK moddef2.stdtxt
harness 'moddef2 X INST=2' FAIL moddef2_2.stdtxt moddef2_2.errtxt
harness 'moddef2 X NAME=cmdline' OK moddef2_cmdline1.stdtxt
harness 'moddef2 X NAME=cmdline mode=disable' OK moddef2_cmdline2.stdtxt
# modref1: Referencing other modules
harness 'modref1' OK modref1_help.stdtxt
harness 'modref1 X' OK modref1.stdtxt
# remote.agm
harness 'remote' OK remote_help.stdtxt
harness 'remote X' OK remote.stdtxt
harness 'remote X mode=simple' OK remote_simple.stdtxt
harness 'remote X mode=complex' OK remote_complex.stdtxt - \
  remote1.tmc ../../remote/TM/rmtsrc.tmc remote2.tmc remote2.src
harness 'errmod1 X' FAIL errmod1.stdtxt errmod1.errtxt
harness 'modref2' OK modref2_help.stdtxt
harness 'modref2 X' FAIL modref2.stdtxt modref2.errtxt
harness 'recurse' OK recurse_help.stdtxt
harness 'recurse X' FAIL recurse.stdtxt recurse.errtxt

harness_ag ex1.spec OK ex1.stdtxt - Makefile ex1.mkf
harness_ag err1.spec FAIL err1.stdtxt err1.errtxt Makefile err1.mkf
