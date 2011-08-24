# Makefile Generated Wed Aug 24 14:49:12 EDT 2011
MNC=OMSdiag
HOMEDIR=/home/nort/BUILD/QNX6Build/QNX6/oms/OMSdiag
SPECFILE=OMSdiag.spec
SRC=base.tmc OMS.tbl OMS.doit
TOOL=OMSdiag.spec Makefile
SCRIPT=interact
OBJ=OMSdiagcol.cc OMSdiag.pcm tm.dac OMSdiagcoloui.c OMSdiagcoloui.o
OBJ+=OMSdiagcol.o OMStbl.tmc OMSdisp.cc OMSdispoui.c OMSdispoui.o
OBJ+=OMSdisp.o OMStblnc.tmc OMSdispnc.cc OMSdispncoui.c
OBJ+=OMSdispncoui.o OMSdispnc.o doit.dep OMSdiagcmd.cc OMSdiagcmd.dat
OBJ+=OMSdiagclt.o OMSdiagcltoui.c OMSdiagcltoui.o OMSdiagcltnc.o
OBJ+=OMSdiagcltncoui.c OMSdiagcltncoui.o OMSdiagsrvr.o
OBJ+=OMSdiagsrvroui.c OMSdiagsrvroui.o
TGT=OMSdiagcol OMSdisp OMSdispnc doit OMSdiagclt OMSdiagcltnc
TGT+=OMSdiagsrvr
DISTRIB=OMSdiag.pcm tm.dac
IDISTRIB=Experiment.config VERSION doit
SRCDIST=OMS.tbl base.tmc
NONRCS=
TMCBASE=base.tmc
CMDBASE=/usr/local/share/huarp/root.cmd
CMDBASE+=/usr/local/share/huarp/getcon.cmd
CMDBASE+=/usr/local/share/huarp/OMS.cmd
RCS=$(SRC) $(TOOL) $(SCRIPT)
SOURCE=$(RCS) $(NONRCS)
OBJECT=$(OBJ) *.err
TARGET=$(TGT)
DSTRB=$(TGT) $(DISTRIB) $(SCRIPT)
TGTDIR= $(TGTNODE)/home/OMSdiag
include /usr/local/share/huarp/appgen.mk

all : Makefile $(TGT)
Makefile : OMSdiag.spec
	appgen OMSdiag.spec
VERSION=`cat VERSION`
extdistribution : $(DSTRB) $(SRCDIST)
	@[ -d $(TGTDIR)/bin ] || mkdir $(TGTDIR)/bin
	@[ -d $(TGTDIR)/bin/$(VERSION) ] || mkdir $(TGTDIR)/bin/$(VERSION)
	@[ -d  $(TGTDIR)/bin/$(VERSION)/Base ] || mkdir $(TGTDIR)/bin/$(VERSION)/Base
	@distribute $(FORCE) $(TGTDIR)/bin/$(VERSION) $(DSTRB)
	@distribute $(FORCE) $(TGTDIR)/bin/$(VERSION)/Base $(SRCDIST)
distribution : extdistribution
	@distribute $(FORCE) $(TGTDIR) $(IDISTRIB)
OMSdiagcol : OMSdiagcol.o OMSdiagcoloui.o
	$(LINK.privCXX) OMSdiagcol.o OMSdiagcoloui.o -ltmpp -ltm -lnort
	$(OUIUSE) OMSdiagcoloui.c
	$(PROMOTE) OMSdiagcol
OMSdisp : OMSdisp.o OMSdispoui.o
	$(LINK.normCXX) OMSdisp.o OMSdispoui.o -ldisp -ltmph -lph -ltmpp -ltm -lnort
	$(OUIUSE) OMSdispoui.c
OMSdispnc : OMSdispnc.o OMSdispncoui.o
	$(LINK.normCXX) OMSdispnc.o OMSdispncoui.o -ldispnc -ltmnc -lncurses -ltmpp -ltm -lnort
	$(OUIUSE) OMSdispncoui.c
doit : OMS.doit
	mkdoit3 -o doit  OMS.doit
OMSdiagclt : OMSdiagclt.o OMSdiagcltoui.o
	$(LINK.normCXX) OMSdiagclt.o OMSdiagcltoui.o -ltmph -lph -ltmpp -ltm -lnort
	$(OUIUSE) OMSdiagcltoui.c
OMSdiagcltnc : OMSdiagcltnc.o OMSdiagcltncoui.o
	$(LINK.normCXX) OMSdiagcltnc.o OMSdiagcltncoui.o -ltmnc -lncurses -ltmpp -ltm -lnort
	$(OUIUSE) OMSdiagcltncoui.c
OMSdiagsrvr : OMSdiagsrvr.o OMSdiagsrvroui.o
	$(LINK.privCXX) OMSdiagsrvr.o OMSdiagsrvroui.o -ltmpp -ltm -lnort
	$(OUIUSE) OMSdiagsrvroui.c
	$(PROMOTE) OMSdiagsrvr
OMSdiagcol.cc : base.tmc
	$(TMC.col) base.tmc
OMSdiagcoloui.c : $(OUIDIR)/collect.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/collect.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/msg.oui
OMSdiagcoloui.o : OMSdiagcoloui.c
OMSdiagcol.o : OMSdiagcol.cc
OMStbl.tmc : OMS.tbl
	$(TABLE) OMS.tbl
OMSdisp.cc : base.tmc /usr/local/share/huarp/flttime.tmc OMStbl.tmc
	$(TMC) base.tmc /usr/local/share/huarp/flttime.tmc OMStbl.tmc
OMSdispoui.c : $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
OMSdispoui.o : OMSdispoui.c
OMSdisp.o : OMSdisp.cc
OMStblnc.tmc : OMS.tbl
	$(NCTABLE) OMS.tbl
OMSdispnc.cc : base.tmc /usr/local/share/huarp/flttime.tmc OMStblnc.tmc
	$(TMC) base.tmc /usr/local/share/huarp/flttime.tmc OMStblnc.tmc
OMSdispncoui.c : $(OUIDIR)/nctable.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/nctable.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
OMSdispncoui.o : OMSdispncoui.c
OMSdispnc.o : OMSdispnc.cc
doit.dep : OMS.doit
	mkdoit3 -M -o doit  OMS.doit
-include doit.dep
OMSdiagcmd.cc : /usr/local/share/huarp/root.cmd /usr/local/share/huarp/getcon.cmd /usr/local/share/huarp/OMS.cmd
	$(CMDGEN) -d OMSdiagcmd.dat /usr/local/share/huarp/root.cmd /usr/local/share/huarp/getcon.cmd /usr/local/share/huarp/OMS.cmd
OMSdiagclt.o : OMSdiagcmd.cc
	$(COMPILE.clt) OMSdiagcmd.cc
OMSdiagcltoui.c : $(OUIDIR)/kbdclt.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/kbdclt.oui $(OUIDIR)/msg.oui
OMSdiagcltoui.o : OMSdiagcltoui.c
OMSdiagcltnc.o : OMSdiagcmd.cc
	$(COMPILE.cltnc) OMSdiagcmd.cc
OMSdiagcltncoui.c : $(OUIDIR)/kbdclt.oui $(OUIDIR)/nctable.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/kbdclt.oui $(OUIDIR)/nctable.oui $(OUIDIR)/msg.oui
OMSdiagcltncoui.o : OMSdiagcltncoui.c
OMSdiagsrvr.o : OMSdiagcmd.cc
	$(COMPILE.srvr) OMSdiagcmd.cc
OMSdiagsrvroui.c : $(OUIDIR)/cis.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/cis.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/msg.oui
OMSdiagsrvroui.o : OMSdiagsrvroui.c

tidy :
	rm -f $(OBJECT)
clean : tidy
	rm -f $(TARGET)
.cvsignore :
	@for i in Makefile '.B4*' $(OBJECT) $(TARGET); do echo "$$i"; done >.cvsignore
