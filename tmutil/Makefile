# QNX6/tmutil
#  This directory holds general-purpose utilities for
#  The Anderson Group Data Acquisition System/QNX6
BIN=TM appgen
SBIN=promote
DATA=appgen.mk
INSTALL=cp -vn
.PHONY : install install-sbin install-bin install-data
install : install-sbin install-bin install-data
install-sbin :
	$(INSTALL) $(SBIN) /usr/local/sbin
install-bin :
	$(INSTALL) $(BIN) /usr/local/bin
install-data :
	$(INSTALL) $(DATA) /usr/local/share/huarp
