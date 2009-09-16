BIN=decipher install_app mkpatch txt2html word2txt \
    dircompare dircksum genui genui_chk con2html
# distill has very limited applicability, so don't
# install by default
NOINST=distill
#INSTALL=/usr/bin/install
INSTALL=cp -v
install :
	$(INSTALL) $(BIN) /usr/local/bin
