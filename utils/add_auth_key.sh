#! /bin/sh
# add_auth_key
# Reads in a public key value from stdin and appends it to ~/.ssh/authorized_keys
function nl_error {
  echo "add_auth_key: $*" >&2
  exit 1
}

function add_trailing_newline {
  filename=$1
  if [ -s $filename ]; then # file is non-empty
    last_char=`od -An -tx1  $filename | tail -n2`
    last_char=${last_char% }
    last_char=${last_char##* }
    if [ ! "X$last_char" = "X0a" ]; then
      echo >>$filename
    fi
  fi
}

ak=~/.ssh/authorized_keys
if [ ! -f $ak ]; then
  # Create it
  [ -d ~ ] || nl_error "No home dir"
  [ -d ~/.ssh ] || mkdir ~/.ssh
  chmod 0755 ~/.ssh
  touch $ak
  chmod 0755 $ak
fi

add_trailing_newline $ak
cat >>$ak
add_trailing_newline $ak
