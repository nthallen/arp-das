#! /bin/sh
# Flight instrument script
#__USAGE
#%C
export PATH=:/bin:/usr/bin:/usr/local/bin
if [ "x$1" != "xteed" ]; then
  touch flight.sh.log
  if [ -w flight.sh.log ]; then
    $0 teed 2>&1 | tee -a flight.sh.log
    exit 0
  fi
  echo "Cannot write to flight.sh.log"
fi
echo "\nRunning flight.sh: \c"; date

#----------------------------------------------------------------
# This is where we will decide what experiment we are
#----------------------------------------------------------------
cfile=Experiment.config
if [ -f "$cfile.$NODE" ]; then
  . $cfile.$NODE
elif [ -f "$cfile" ]; then
  . $cfile
else
  echo flight.sh: missing $cfile >&2
fi

if [ -n "$Experiment" -a -n "$HomeDir" -a -d "$HomeDir" ]; then
  cd $HomeDir
else
  echo flight.sh: Experiment or HomeDir undefined or non-existant >&2
  Experiment=none
fi
export Experiment

typeset launch_error=""

function Launch {
  name=$1
  shift
  [ -n "$launch_error" ] && return 1
  [ -n "$VERBOSE" ] && echo "Launch: $*"
  if { $* & }; then
    if [ "$name" != "-" ]; then
      [ "${name#/}" = "$name" ] && name="/dev/huarp/$Experiment/$name"
      [ -n "$VERBOSE" ] && echo "Launch: Waiting for $!:$name"
      waitfor $name 10 || {
        echo "Launch namewait failure: $*" >&2
        launch_error=yes
        return 1
      }
    fi
  else
    echo "Launch Error: $*" >&2
    launch_error=yes
    return 1
  fi
  return 0
}

umask g+w

if [ -n "$SUBBUSD" -a ! -e /dev/huarp/subbus ]; then
  launch /dev/huarp/subbus subbusd_$SUBBUSD -V
fi
VERSION=1.0
[ -f VERSION ] && VERSION=`cat VERSION`
if [ -d bin/$VERSION/ ]; then
  TMBINDIR=`fullpath -t bin/$VERSION/`
  PATH=$TMBINDIR:$PATH
else
  TMBINDIR='.'
fi
export TMBINDIR
if [ -n "$SCRIPT_OVERRIDE" -a -r "$SCRIPT_OVERRIDE" ]; then
  script=`cat $SCRIPT_OVERRIDE`
  rm $SCRIPT_OVERRIDE
elif [ -n "$PICKFILE" ]; then
  script=`cd $TMBINDIR; $PICKFILE`
else
  script=${RUNFILE:-runfile.dflt}
fi
[ ${script#/} = "$script" ] && script=$TMBINDIR/$script
if [ -x $script ]; then
  echo flight.sh: `id`: Experiment=$Experiment script=$script
  . $script
else
  echo flight.sh: Specified script $script not found >&2
fi
# [ -z "$launch_error" ] && pick_file -q

typeset qoc
[ -z "$FlightNode" ] && qoc="-q"
exec parent $qoc
