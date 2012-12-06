SRC=qclitest.c qcliutil.c qcliutil.h
TOOL=Makefile todo
#TARGET=qclid qcliprog qclitest chktime qcliread icoswave.out
TARGET=qclisd qclisprog qclistest qclidacsd qclidprog qclidtest
INCLUDE_HEADERS=qclid.h
DATA_HEADERS=qcli.cmd
CFLAGS=-Wall -g -DEXTRA_CHECKS
LDFLAGS=-L/usr/local/lib

include /usr/local/share/huarp/appgen.mk
LIBS=-lnort -ltm

all : $(TARGET)

SRVROBJ=qclisrvr.o qclidoui.o qcliutil.o qclisb.o
qclid : $(SRVROBJ)
	$(LINK.priv) $(SRVROBJ) $(LIBS)
	usemsg qclid qclidoui.c
	promote qclid
qclisrvr.o : qclisrvr.c qcliutil.h
qclidoui.c : qclid.oui qclisb.oui
	$(OUI) qclid.oui qclisb.oui

SSRVROBJ= qclisrvr.o qclisdoui.o qcliutil.o qcliser.o
qclisd : $(SSRVROBJ)
	$(LINK.norm) $(SSRVROBJ) $(LIBS) -ltm
	usemsg qclisd qclisdoui.c
qclisdoui.c : qclid.oui qcliser.oui
	$(OUI) qclid.oui qcliser.oui

DSRVROBJ= qclisrvr.o qclidacsdoui.o qcliutil.o qclidacs.o
qclidacsd : $(DSRVROBJ)
	$(LINK.norm) $(DSRVROBJ) $(LIBS) -ltm -lsubbus
	usemsg qclidacsd qclidacsdoui.c
qclidacsdoui.c : qclid.oui qclidacs.oui
	$(OUI) qclid.oui qclidacs.oui

qcliprog : qcliprog.o qcliutil.o qclisb.o qcliprogoui.o qcliver.o
	$(LINK.priv) qcliprog.o qcliutil.o qclisb.o qcliprogoui.o qcliver.o $(LIBS)
	usemsg qcliprog qcliprogoui.c
	promote qcliprog
qcliprog.o : qcliprog.c qcliutil.h
qcliprogoui.c : qcliprog.oui qclisb.oui
	$(OUI) qcliprog.oui qclisb.oui

qclisprog : qcliprog.o qcliutil.o qcliser.o qclisprogoui.o qcliver.o
	$(LINK.norm) qcliprog.o qcliutil.o qcliser.o qclisprogoui.o qcliver.o $(LIBS)
	usemsg qclisprog qclisprogoui.c
qclisprogoui.c : qcliprog.oui qcliser.oui
	$(OUI) qcliprog.oui qcliser.oui

qclidprog : qcliprog.o qcliutil.o qclidacs.o qclidprogoui.o
	$(LINK.norm) qcliprog.o qcliutil.o qclidacs.o qclidprogoui.o $(LIBS) -lsubbus
	usemsg qclidprog qclidprogoui.c
qclidprogoui.c : qcliprog.oui qclidacs.oui
	$(OUI) qcliprog.oui qclidacs.oui

qclitest : qclitest.o qcliutil.o qclisb.o qclitestoui.o
	$(LINK.priv) qclitest.o qcliutil.o qclisb.o qclitestoui.o $(LIBS)
	usemsg qclitest qclitestoui.c
	promote qclitest
qclitest.o : qclitest.c qcliutil.h
qclitestoui.c : qclisb.oui
	$(OUI) qclisb.oui

qclistest : qclitest.o qcliutil.o qcliser.o qclistestoui.o
	$(LINK.norm) qclitest.o qcliutil.o qcliser.o qclistestoui.o $(LIBS)
	usemsg qclistest qclistestoui.c
qclistestoui.c : qcliser.oui
	$(OUI) qcliser.oui

qclidtest : qclitest.o qcliutil.o qclidacs.o qclidtestoui.o
	$(LINK.norm) qclitest.o qcliutil.o qclidacs.o qclidtestoui.o $(LIBS) -lsubbus
	usemsg qclidtest qclidtestoui.c
qclidtestoui.c : qclidacs.oui
	$(OUI) qclidacs.oui

qcliread : qcliread.o qcliutil.o
	$(LINK.priv) qcliread.o qcliutil.o $(LIBS)
	usemsg qcliread qcliread.c
	promote qcliread
qcliread.o : qcliread.c qcliutil.h

icoswave.out : icoswave.qcli
	qclicomp icoswave.qcli >icoswave.out

.PHONY : clean all install install-bin install-include install-data
clean :
	rm -f $(TARGET) *.o *oui.c FullBuild.*


install : install-bin install-include install-data

install-bin : $(TARGET)
	@cp -vn $(TARGET) $(TGTNODE)/usr/local/bin

install-include :
	@cp -vn $(INCLUDE_HEADERS) $(TGTNODE)/usr/local/include

install-data :
	@cp -vn $(DATA_HEADERS) $(TGTNODE)/usr/local/share/huarp
