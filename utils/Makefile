# QNX6/utils
# This directory contains miscellaneous utilities that need to be
# compiled.
BIN=unsu vmutil FullBuild
SBIN=pkg_copy
DATA=
OSDATA=config.site
INSTALL=cp -vn
PREFIX=$(TGTNODE)/usr/local

.PHONY : all install install-sbin install-bin install-osdata install-data
all : install
install : install-bin install-osdata install-sbin
install-sbin : $(SBIN)
	mkdir -p $(PREFIX)/sbin
	@$(INSTALL) $(SBIN) $(PREFIX)/sbin
install-bin : $(BIN)
	mkdir -p $(PREFIX)/bin
	@$(INSTALL) $(BIN) $(PREFIX)/bin
install-data : $(DATA)
	mkdir -p $(PREFIX)/share/huarp
	@$(INSTALL) $(DATA) $(PREFIX)/share/huarp
install-osdata : $(OSDATA)
	mkdir -p $(PREFIX)/share
	@$(INSTALL) $(OSDATA) $(PREFIX)/share
#install-oui : $(OUI)
#	@mkdir -p $(PREFIX)/share/oui
#	@$(INSTALL) $(OUI) $(PREFIX)/share/oui
