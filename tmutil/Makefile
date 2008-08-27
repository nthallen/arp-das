# QNX6/tmutil
#  This directory holds general-purpose utilities for
#  The Anderson Group Data Acquisition System/QNX6
BIN=TM appgen distribute parent
SBIN=promote flight.sh
DATA=appgen.mk
INSTALL=cp -vn
CFLAGS=-Wall -g -I/usr/local/include
LDFLAGS=-L/usr/local/lib -Wl,-rpath -Wl,/usr/local/lib

.PHONY : all install install-sbin install-bin install-data
all : parent
install : install-sbin install-bin install-data
install-sbin :
	@$(INSTALL) $(SBIN) /usr/local/sbin
install-bin :
	@$(INSTALL) $(BIN) /usr/local/bin
install-data :
	@$(INSTALL) $(DATA) /usr/local/share/huarp
parent : parent.o parentoui.o
	$(LINK.c) -o parent parent.o parentoui.o -lnort
	usemsg parent parentoui.c
parentoui.c : parent.oui
	oui -o parentoui.c parent.oui
