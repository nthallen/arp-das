#! /bin/sh
# Flight instrument script
#__USAGE
#%C
export PATH=:/bin:/usr/bin:/usr/local/bin

if [ -f OSUPDATEREQ ]; then
  echo "flight.sh: suspending due to OSUPDATEREQ" |
    tee -a flight.sh.log
  while [ -f OSUPDATEREQ ]; do
    sleep 1
  done
fi

if [ "x$1" != "xteed" ]; then
  touch flight.sh.log
  if [ -w flight.sh.log ]; then
    exec 5>&1
    $0 teed 4>&5 2>&1 | tee -a flight.sh.log
    exit 0
  fi
  echo "Cannot write to flight.sh.log"
  exec 4>&1
fi
echo "\nRunning flight.sh: \c"; date

#----------------------------------------------------------------
# This is where we will decide what experiment we are
#----------------------------------------------------------------
cfile=Experiment.config
if [ -f "$cfile" -a -r "$cfile" ]; then
  . $cfile
else
  echo flight.sh: missing or unreadable $cfile: stopping >&2
  exec 1>&4 2>&4 # close pipe to flight.sh.log and reopen stdout/err
  exec parent
fi

if [ -n "$Experiment" -a -n "$HomeDir" -a -d "$HomeDir" ]; then
  cd $HomeDir
else
  echo Experiment or HomeDir undefined or non-existant >&2
  Experiment=none
fi
export Experiment

umask g+w

typeset launch_error=""
typeset parent_loop=''
typeset script=''

[ -n "$LOOP_STOP_FILE" ] || LOOP_STOP_FILE=loopstop.txt
[ -n "$LOOP_START_FILE" ] || LOOP_START_FILE=loopstart.txt
if [ -n "$LOOP_ON_FILE" -a -e "$LOOP_ON_FILE" ]; then
  echo "`date '+%T'` Invoking reduce for LOOP_ON_FILE"
  [ -z "$REDUCE_LOG_FILE" ] && REDUCE_LOG_FILE=reducelog.txt
  exec 1>&4 2>&4 # close pipe to flight.sh.log and reopen stdout/err
  BEDTIME=yes reduce 2>&1 | tee -a $REDUCE_LOG_FILE
  if [ -e "$LOOP_ON_FILE" ]; then
    echo "`date '+%T'` [ERROR] reduce did not clear LOOP_ON_FILE '$LOOP_ON_FILE'" |
      tee -a $REDUCE_LOG_FILE
    rm -f $LOOP_STOP_FILE $LOOP_START_FILE
    exec parent
  fi
  [ -f $LOOP_STOP_FILE ] && mv $LOOP_STOP_FILE $LOOP_START_FILE
  exit 0 # exit to reestablish tee log
elif [ -e "$LOOP_STOP_FILE" ]; then
  if [ -n "$RESTART_FILE" -a -e "$RESTART_FILE" ]; then
    echo "`date '+%T'` Restarting"
    rm -f "$RESTART_FILE"
    script=`cat $LOOP_STOP_FILE`
  else
    echo "`date '+%T'` Looping terminated"
    rm -f $LOOP_STOP_FILE $LOOP_START_FILE
    script=/dev/null
  fi
elif [ -e "$LOOP_START_FILE" ]; then
  script=`cat $LOOP_START_FILE`
  rm -f $LOOP_START_FILE
fi

function Launch {
  name=$1
  shift
  [ -n "$launch_error" ] && return 1
  [ -n "$VERBOSE" ] && echo "`date +%T` [DEBUG] Launch: $*"
  if { $* & }; then
    Launch_pid=$!
    echo "`date +%T` Launch: $Launch_pid $*"
    if [ "$name" = "DG/cmd" ]; then
      parent_loop="-q -M $Launch_pid -t 5"
    fi
    if [ "$name" != "-" ]; then
      [ "$name" = "-DC-" ] && name="/var/huarp/run/$Experiment/$!"
      [ "${name#/}" = "$name" ] && name="/dev/huarp/$Experiment/$name"
      [ -n "$VERBOSE" ] &&
        echo "`date +%T` [DEBUG] Launch: Waiting for $Launch_pid:$name"
      waitfor $name 10 || {
        echo "`date +%T` [ERROR] Launch: namewait failure: $*"
        launch_error=yes
        return 1
      }
    fi
  else
    echo "`date +%T` [ERROR] Launch: $*"
    launch_error=yes
    return 1
  fi
  return 0
}

SUBBUS_PID=''
if [ -n "$SUBBUSD" -a ! -e /dev/huarp/subbus ]; then
  if [ -n "$SUBBUSD_DELAY" ]; then
    echo "`date +%T` sleep $SUBBUSD_DELAY for subbus"
    sleep $SUBBUSD_DELAY
  fi
  Launch /dev/huarp/subbus subbusd_$SUBBUSD -V
  [ -z "$launch_error" ] && SUBBUS_PID=$Launch_pid
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

if [ -z "$script" -a -n "$SCRIPT_OVERRIDE" ]; then
  if [ "${SCRIPT_OVERRIDE#/}" = "$SCRIPT_OVERRIDE" -a -n "$HomeDir" ]; then
    [ -n "$EXP_NODES" ] || EXP_NODES=`hostname`
    if [ -n "$FlightNode" ]; then
      [ -z "$NETWORK_DELAY" ] && NETWORK_DELAY=3
      if [ $NETWORK_DELAY -gt 0 ]; then
        echo "`date +%T` sleep $NETWORK_DELAY to acquire network connections"
        sleep $NETWORK_DELAY
      fi
    fi
    for node in $EXP_NODES; do
      sfile=/net/$node$HomeDir/$SCRIPT_OVERRIDE
      if [ -r $sfile ]; then
        script=`cat $sfile`
        rm $sfile
        echo "`date +%T` Script name '$script' read from $sfile"
      else
        [ -n "$VERBOSE" ] && echo "`date +%T` [DEBUG] $sfile not found"
      fi
    done
  elif [ -n "$SCRIPT_OVERRIDE" -a -r "$SCRIPT_OVERRIDE" ]; then
    script=`cat $SCRIPT_OVERRIDE`
    rm $SCRIPT_OVERRIDE
  fi
fi
if [ -z "$script" ]; then
  if [ -n "$PICKFILE" ]; then
    script=`cd $TMBINDIR; $PICKFILE`
  else
    script=${RUNFILE:-runfile.dflt}
  fi
fi
  
[ ${script#/} = "$script" ] && script=$TMBINDIR/$script
if [ -r $script ]; then
  echo "`date +%T` `id`: Experiment=$Experiment script=$script"
  . $script
else
  echo "`date +%T` Specified script $script not found"
fi

if [ -n "$SUBBUS_PID" ]; then
  pids=`jobs -p | grep -v $SUBBUS_PID`
else
  pids=`jobs -p`
fi

if [ -n "$launch_error" -o -n "$pids" ]; then
  [ -n "$FlightNode" ] && echo $script >$LOOP_STOP_FILE
else
  [ -n "$SUBBUS_PID" ] && kill -SIGHUP $SUBBUS_PID
  echo "`date +%T` No subprocesses, closing flight.sh.log"
  exec 1>&4 2>&4
fi

[ -z "$FlightNode" -a -z "$parent_loop" ] && parent_loop="-q"
exec parent $parent_loop
