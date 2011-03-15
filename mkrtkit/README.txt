mkrtkit
mkrtkitarch

These scripts provide tools necessary to build an install CD from a full
installation system. The package hierarchy defined here can also be used
to keep systems up to date.

The assumption is that all of these tools are run against a system that
is a full licensed installation of the QNX development system. The target
systems are runtime systems, which may include some free open source
development tools.

Package definitions are stored under the pkgs directory. Each package
contains a Header file and one or more MANIFEST files.

The Header may include comments, dependencies and reference to the
MANIFEST files. Comments appear on lines starting with the '#' character.
Dependencies are specified with the 'Requires:' keyword and list
the names of one or more packages. The 'Files:' keyword identifies
the MANIFEST files:

  Files: <manifest> @ <root>

<manifest> is the name of the MANIFEST file. <root> is a path prefix
which, when applied to the files listed in the MANIFEST file, identifies
the source location of the files. The path as listed in the MANIFEST
is where the files will reside on the target system in all cases.

MANIFEST files should include listings for any directories that are
referenced and must explicitly identify every file to be included.
Earlier versions took advantage of tar's recursive behavior to
reference complete directory subtrees, but that made it impossible
to identify the directories themselves without including the
subtree.

In development directories, the file mkrtkit.conf identifies files
from that directory that should be included in one or more packages.

UPDATE STRATEGY

Run mkrtkit_chkconfs to make sure any new files have been added to
appropriate MANIFESTS.

Run chkpkgs
  Currently just checks that all the files listed in MANIFEST files
  exist. Need to add these version checks.
  Read Header to determine Version, list of MANIFESTS
  If current version is installed, compare MANIFESTS with installed
  MANIFESTS and check the modify time of listed files against the
  existing archive. As soon as an update is required, we can
  stop comparing.
  Check MANIFESTS to make sure all listed files exist
Run install
  Install should not overwrite an installation
  Install should create the tar.gz file

An archive needs to be rebuilt if:
  a) The version number has changed (the archive does not exist)
  b) The list of files has changed since the last build
  c) Any of the files have changed since the last build
How exactly we determine b and c is a little tricky.
b) is relatively easy. Our choices are:
  1) compare the date of the MANIFESTs against the date of
     the archive. This can lead to false positives if the
     date has been updated but the contents haven't changed.
  2) compare the contents of the MANIFESTs to the installed
     copies. This can also lead to false positives if only
     the order of files has changed.
  3) compare the sorted contents.
c) We probably have to go with comparing the file dates
   to the archive date. This will lead to false positives
   in the case where a package was rebuilt and reinstalled
   without any real changes, but that should be OK.

A check utility should be run against each package. If b or c
is true, it should report that a new version is required. The
user will have to manually modify the version number in the
Header.

The install should create the archive in addition to copying
the Header and MANIFESTs into /var/huarp/pkg.

Bootstrap packages are slightly different in that they
do not include any of their own files (no MANIFESTs), just
identify other packages that are required. In this case (only)
the version should be bumped if any of the required packages
has been updated. mkrtkitarch should be responsible for that.
