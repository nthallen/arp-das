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

