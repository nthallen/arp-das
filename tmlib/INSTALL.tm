After downloading from the CVS repository:

  aclocal                # to generate aclocal.m4
  automake --add-missing # to generate Makefile.in src/Makefile.in
                         # May need to copy in ltmain.sh from
                         # /usr/local/share/libtool
  autoconf               # to generate configure
  ./configure            # to generate Makefile src/Makefile
  make
  make install

