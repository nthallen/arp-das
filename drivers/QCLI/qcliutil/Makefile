SRC=qclitest.c qcliutil.c qcliutil.h
TOOL=Makefile todo
#TARGET=qclid qcliprog qclitest chktime qcliread icoswave.out
TARGET=qclisd qclisprog qclistest

include /usr/local/share/huarp/appgen.mk
LIBFLAGS=-lnort

SRVROBJ=qclisrvr.o qclidoui.o qcliutil.o qclisb.o
qclid : $(SRVROBJ)
	$(LINK.priv) $(SRVROBJ)
	usemsg qclid qclidoui.c
	promote qclid
qclisrvr.o : qclisrvr.c qcliutil.h /usr/local/include/qcliclt.h
qclidoui.c : qclid.oui qclisb.oui
	$(OUI) qclid.oui qclisb.oui

SSRVROBJ= qclisrvr.o qclisdoui.o qcliutil.o qcliser.o
qclisd : $(SSRVROBJ)
	$(LINK.norm) $(SSRVROBJ)
	usemsg qclisd qclisdoui.c
qclisdoui.c : qclid.oui qcliser.oui
	$(OUI) qclid.oui qcliser.oui

qcliprog : qcliprog.o qcliutil.o qclisb.o qcliprogoui.o
	$(LINK.priv) qcliprog.o qcliutil.o qclisb.o qcliprogoui.o
	usemsg qcliprog qcliprogoui.c
	promote qcliprog
qcliprog.o : qcliprog.c qcliutil.h
qcliprogoui.c : qcliprog.oui qclisb.oui
	$(OUI) qcliprog.oui qclisb.oui

qclisprog : qcliprog.o qcliutil.o qcliser.o qclisprogoui.o
	$(LINK.norm) qcliprog.o qcliutil.o qcliser.o qclisprogoui.o
	usemsg qclisprog qclisprogoui.c
qclisprogoui.c : qcliprog.oui qcliser.oui
	$(OUI) qcliprog.oui qcliser.oui

qclitest : qclitest.o qcliutil.o qclisb.o qclitestoui.o
	$(LINK.priv) qclitest.o qcliutil.o qclisb.o qclitestoui.o
	usemsg qclitest qclitestoui.c
	promote qclitest
qclitest.o : qclitest.c qcliutil.h
qclitestoui.c : qclisb.oui
	$(OUI) qclisb.oui

qclistest : qclitest.o qcliutil.o qcliser.o qclistestoui.o
	$(LINK.norm) qclitest.o qcliutil.o qcliser.o qclistestoui.o
	usemsg qclistest qclistestoui.c
qclistestoui.c : qcliser.oui
	$(OUI) qcliser.oui

qcliread : qcliread.o qcliutil.o
	$(LINK.priv) qcliread.o qcliutil.o
	usemsg qcliread qcliread.c
	promote qcliread
qcliread.o : qcliread.c qcliutil.h

icoswave.out : icoswave.qcli
	qclicomp icoswave.qcli >icoswave.out

clean :
	rm -f $(TARGET) *.o

all : $(TARGET)
