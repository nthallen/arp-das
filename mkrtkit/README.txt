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
