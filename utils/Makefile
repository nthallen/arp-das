# QNX6/utils
# This directory contains miscellaneous utilities that need to be
# compiled.
BIN=unsu vmutil
SBIN=mkrtkit mkrtkitarch
DATA=rtinstall
INSTALL=cp -vn

.PHONY : all install install-sbin install-bin install-data
all : install
install : install-sbin install-bin install-data
install-sbin : $(SBIN)
	mkdir -p $(TGTNODE)/usr/local/sbin
	$(INSTALL) $(SBIN) $(TGTNODE)/usr/local/sbin
install-bin : $(BIN)
	mkdir -p $(TGTNODE)/usr/local/bin
	$(INSTALL) $(BIN) $(TGTNODE)/usr/local/bin
install-data : $(DATA)
	mkdir -p $(TGTNODE)/usr/local/share/huarp
	$(INSTALL) $(DATA) $(TGTNODE)/usr/local/share/huarp
#install-oui : $(OUI)
#	@mkdir -p $(TGTNODE)/usr/local/share/oui
#	@$(INSTALL) $(OUI) $(TGTNODE)/usr/local/share/oui
