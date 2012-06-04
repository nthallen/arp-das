# QNX6/tmutil
#  This directory holds general-purpose utilities for
#  The Anderson Group Data Acquisition System/QNX6
BIN=TM appgen distribute parent saverun fixdisk mkinp tomat extract
BIN+=mkcvsignore mkdoit3 mkexpdir reduce settime flttime chk_display
BIN+=chk_extract CDimage prepCD swscomp
SBIN=promote flight.sh signal_parent getcon
DATA=appgen.mk edf2ext.awk edf2oui.awk Makefile.pm
DATA+=nulltbl.tmc vl_temp.tmc vl_temp_col.tmc
DATA+=freemem.tmc freemem_col.tmc
OUI=seteuid.oui
INSTALL=cp -vn
CFLAGS=-Wall -g -I/usr/local/include
CXX=cc
CXXFLAGS=-Wall -g -I/usr/local/include -lang-c++
LDFLAGS=-L/usr/local/lib -Wl,-rpath -Wl,/usr/local/lib

.PHONY : all install install-sbin install-bin install-data
.PHONY : checksudo clean

all : parent getcon
install : install-sbin install-bin install-data install-oui
install-sbin : checksudo
	chmod +x $(SBIN)
	@$(INSTALL) $(SBIN) $(TGTNODE)/usr/local/sbin
install-bin : checksudo
	chmod +x $(BIN)
	@$(INSTALL) $(BIN) $(TGTNODE)/usr/local/bin
install-data :
	@$(INSTALL) $(DATA) $(TGTNODE)/usr/local/share/huarp
install-oui :
	@$(INSTALL) $(OUI) $(TGTNODE)/usr/local/share/oui
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

getcon : getcon.o getconoui.o
	$(LINK.c) -o getcon getcon.o getconoui.o -lnort -ltm
	usemsg getcon getconoui.c
getconoui.c : getcon.oui
	oui -o getconoui.c getcon.oui

# Trivial check for the existence of sudo where these utilities
# expect to find it
checksudo :
	/usr/pkg/bin/sudo -V >/dev/null
clean :
	rm -f FullBuild.* *.o getcon DCtab parent
