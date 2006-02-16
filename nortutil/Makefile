BIN=decipher install_app mkpatch pic2html2 picscale txt2html word2txt \
    dircompare dircksum genui distill pic2html3 OrchClt
LIB=*.tmpl
INSTALL=/usr/bin/install
#INSTALL=cp -v
install :
	$(INSTALL) $(BIN) /usr/local/bin
	@ [ -d /usr/local/lib/pic2html ] || mkdir /usr/local/lib/pic2html
	$(INSTALL) $(LIB) /usr/local/lib/pic2html
