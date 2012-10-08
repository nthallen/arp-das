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
if [ -f "$cfile" ]; then
  . $cfile
else
  echo flight.sh: missing $cfile: stopping >&2
  exec parent
fi

if [ -n "$Experiment" -a -n "$HomeDir" -a -d "$HomeDir" ]; then
  cd $HomeDir
else
  echo flight.sh: Experiment or HomeDir undefined or non-existant >&2
  Experiment=none
fi
export Experiment

umask g+w

typeset launch_error=""
typeset Looping=no
typeset parent_loop=''
[ -n "$FlightNode" -a -n "$LOOP_ON_FILE" ] && Looping=yes

if [ "$Looping" = yes ]; then
  [ -n "$LOOP_STOP_FILE" ] || LOOP_STOP_FILE=loopstop.txt
  if [ -e "$LOOP_ON_FILE" ]; then
    echo "`date '+%F %T'`: Invoking reduce for LOOP_ON_FILE"
    BEDTIME=yes reduce
    if [ -e "$LOOP_ON_FILE" ]; then
      echo "`date '+%F %T'`: reduce did not clear LOOP_ON_FILE '$LOOP_ON_FILE'"
      rm -f $LOOP_STOP_FILE
      exec parent
    fi
  elif [ -e "$LOOP_STOP_FILE" ]; then
    echo "`date '+%F %T'`: Looping terminated"
    rm -f $LOOP_STOP_FILE
    exec parent
  elif [ -n "$SCRIPT_OVERRIDE" ]; then
    if [ -e "$SCRIPT_OVERRIDE" ]; then
      echo "`date '+%F %T'`: Interactive invocation"
    else
      echo "`date '+%F %T'`: Bootup invocation"
      exec parent
    fi
  fi
  rm -f $LOOP_STOP_FILE
fi

function Launch {
  name=$1
  shift
  [ -n "$launch_error" ] && return 1
  [ -n "$VERBOSE" ] && echo "Launch: $*"
  if { $* & }; then
    if [ $Looping = yes -a "$name" = "cmd/server" ]; then
      parent_loop="-q -M $! -t 5"
      touch $LOOP_STOP_FILE
    fi
    if [ "$name" != "-" ]; then
      [ "$name" = "-DC-" ] && name="/var/huarp/run/$Experiment/$!"
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

if [ -n "$SUBBUSD" -a ! -e /dev/huarp/subbus ]; then
  Launch /dev/huarp/subbus subbusd_$SUBBUSD -V
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
if [ -r $script ]; then
  echo flight.sh: `id`: Experiment=$Experiment script=$script
  . $script
else
  echo flight.sh: Specified script $script not found >&2
fi
# [ -z "$launch_error" ] && pick_file -q

typeset qoc
[ -z "$FlightNode" ] && qoc="-q"
exec parent $qoc $parent_loop
