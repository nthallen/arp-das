# QNX6/tmutil
#  This directory holds general-purpose utilities for
#  The Anderson Group Data Acquisition System/QNX6
BIN=TM appgen distribute parent saverun fixdisk mkinp tomat
SBIN=promote flight.sh signal_parent
DATA=appgen.mk edf2ext.awk edf2oui.awk
INSTALL=cp -vn
CFLAGS=-Wall -g -I/usr/local/include
CXX=cc
CXXFLAGS=-Wall -g -I/usr/local/include -lang-c++
LDFLAGS=-L/usr/local/lib -Wl,-rpath -Wl,/usr/local/lib

.PHONY : all install install-sbin install-bin install-data
all : parent
install : install-sbin install-bin install-data
install-sbin :
	@$(INSTALL) $(SBIN) $(TGTNODE)/usr/local/sbin
install-bin :
	@$(INSTALL) $(BIN) $(TGTNODE)/usr/local/bin
install-data :
	@$(INSTALL) $(DATA) $(TGTNODE)/usr/local/share/huarp
parent : parent.o parentoui.o
	$(LINK.c) -o parent parent.o parentoui.o -lnort
	usemsg parent parentoui.c
parentoui.c : parent.oui
	oui -o parentoui.c parent.oui
DCtap : DCtap.o DCtapoui.o
	$(LINK.c) -o DCtap DCtap.o DCtapoui.o -ltmpp -ltm -lnort -lang-c++
	usemsg DCtap DCtapoui.c
DCtap.o : DCtap.cc
DCtapoui.c : DCtap.oui
	oui -o DCtapoui.c DCtap.oui
