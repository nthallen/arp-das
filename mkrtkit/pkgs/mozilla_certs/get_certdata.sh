#! /bin/sh
# Downloads a current copy of certdata.txt from the mozilla project
# This file should replace the one at
#  /usr/pkg/share/mozilla-rootcerts/certdata.txt.
URL=https://hg.mozilla.org/projects/nss/raw-file/tip/lib/ckfw/builtins/certdata.txt
curl -o certdata.txt $URL
