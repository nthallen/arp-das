BIN=decipher install_app mkpatch txt2html word2txt \
    dircompare dircksum genui genui_chk con2html
# distill has very limited applicability, so don't
# install by default
NOINST=distill
#INSTALL=/usr/bin/install
INSTALL=cp -v

.PHONY : install checkperl

install : checkperl
	$(INSTALL) $(BIN) /usr/local/bin

# This crude Makefile checks for the existence of
# /usr/bin/perl, since the scripts in this directory
# contain #! /usr/bin/perl
# You either need to establish a link from there to
# the real perl, or modify the shebang lines in
# each script.
checkperl :
	/usr/bin/perl -v
