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
checkperl :
	/usr/bin/perl -v
