mozilla_certs package.

The contents of this package are based on the mozilla-rootcerts
pkgsrc package. That package includes a script to extract .pem
files from the certdata.txt file in the mozilla source tree.
This package just wraps up the extracted .pem files along with
a configuration file for subversion to use the files.

There is no source archive for any of these files.

/etc/subversion/servers:

  The non-comment portions of this file consist of the following:

  [groups]

  [global]

  ssl-authority-files = ...

where ... is a semicolon-separated list of the .pem files in
/etc/openssl/certs.

UPDATE PROCEDURE:

# Obtain a current copy of certdata.txt, save in
#   /usr/pkg/share/mozilla-rootcerts/certdata.txt.
# This could fail if the old certs are too out of date. In that
# case, do this under another OS and copy
URL=https://hg.mozilla.org/projects/nss/raw-file/tip/lib/ckfw/builtins/certdata.txt
curl -o certdata.txt $URL
# Now cd /usr/pkg/share/mozilla-rootcerts, save the old one and copy in
# the new one.

# Now as root, do the following:
cd /etc/openssl/certs
mkdir save
mv *.pem save
/usr/pkg/sbin/mozilla-rootcerts extract

# Then come back here and update MANIFEST_certs
cd mkrtkit/pkgs/mozilla_certs
./gen_MANIFEST_certs

# Update /etc/subversion/servers: replace ^ssl-authority-files =.*$ with
#  ssl-authority-files = join(';', </etc/openssl/certs/*.pem>)
./patch_servers.pl /etc/subversion/servers >servers
sudo cp -f servers /etc/subversion/servers
rm servers

# And now update version in Header
