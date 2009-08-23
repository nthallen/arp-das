# QNX6/utils
# This directory contains miscellaneous utilities that need to be
# compiled.
BIN=unsu vmutil
SBIN=mkrtkit mkrtkitarch
DATA=rtinstall
INSTALL=cp -vn

.PHONY : all install install-sbin install-bin install-data
all : install
install : install-sbin install-bin install-data install-oui
install-sbin : $(SBIN)
	@$(INSTALL) $(SBIN) $(TGTNODE)/usr/local/sbin
install-bin : $(BIN)
	@$(INSTALL) $(BIN) $(TGTNODE)/usr/local/bin
install-data : $(DATA)
	@$(INSTALL) $(DATA) $(TGTNODE)/usr/local/share/huarp
#install-oui : $(OUI)
#	@$(INSTALL) $(OUI) $(TGTNODE)/usr/local/share/oui
