# Makefile Generated Thu Oct 22 15:02:15 EDT 2009
MNC=idx64
HOMEDIR=/home/idx64
SPECFILE=idx64.spec
SRC=base.tmc idx64.tmc idx64col.tmc idx64flag.tmc idx64.tbl
TOOL=idx64.spec Makefile
SCRIPT=doit interact
OBJ=idx64col.cc idx64.pcm tm.dac idx64coloui.c idx64coloui.o
OBJ+=idx64col.o idx64tbl.tmc idx64disp.cc idx64dispoui.c
OBJ+=idx64dispoui.o idx64disp.o idx64cmd.c idx64cmd.dat idx64clt.o
OBJ+=idx64cltoui.c idx64cltoui.o idx64srvr.o idx64srvroui.c
OBJ+=idx64srvroui.o
TGT=idx64col idx64disp idx64clt idx64srvr
DISTRIB=idx64.pcm tm.dac
IDISTRIB=Experiment.config VERSION doit
SRCDIST=base.tmc idx64.tmc /usr/local/share/huarp/flttime.tmc
NONRCS=
TMCBASE=base.tmc idx64.tmc /usr/local/share/huarp/flttime.tmc
CMDBASE=/usr/local/share/huarp/root.cmd
CMDBASE+=/usr/local/share/huarp/idx64.cmd
RCS=$(SRC) $(TOOL) $(SCRIPT)
SOURCE=$(RCS) $(NONRCS)
OBJECT=$(OBJ) *.err
TARGET=$(TGT)
DSTRB=$(TGT) $(DISTRIB) $(SCRIPT)
TGTDIR= /home/idx64
include /usr/local/share/huarp/appgen.mk

all : Makefile $(TGT)
Makefile : idx64.spec
	appgen idx64.spec
VERSION=`cat VERSION`
extdistribution : $(DSTRB) $(SRCDIST)
	@[ -d $(TGTDIR)/bin ] || mkdir $(TGTDIR)/bin
	@[ -d $(TGTDIR)/bin/$(VERSION) ] || mkdir $(TGTDIR)/bin/$(VERSION)
	@[ -d  $(TGTDIR)/bin/$(VERSION)/Base ] || mkdir $(TGTDIR)/bin/$(VERSION)/Base
	@distribute $(FORCE) $(TGTDIR)/bin/$(VERSION) $(DSTRB)
	@distribute $(FORCE) $(TGTDIR)/bin/$(VERSION)/Base $(SRCDIST)
distribution : extdistribution
	@distribute $(FORCE) $(TGTDIR) $(IDISTRIB)
idx64col : idx64col.o idx64coloui.o
	$(LINK.privCXX) idx64col.o idx64coloui.o -lsubbus -ltmpp -ltm -lnort
	$(OUIUSE) idx64coloui.c
	$(PROMOTE) idx64col
idx64disp : idx64disp.o idx64dispoui.o
	$(LINK.normCXX) idx64disp.o idx64dispoui.o -ldisp -ltmph -lph -ltmpp -ltm -lnort
	$(OUIUSE) idx64dispoui.c
idx64clt : idx64clt.o idx64cltoui.o
	$(LINK.norm) idx64clt.o idx64cltoui.o -ltmph -lph -ltm -lnort
	$(OUIUSE) idx64cltoui.c
idx64srvr : idx64srvr.o idx64srvroui.o
	$(LINK.priv) idx64srvr.o idx64srvroui.o -ltm -lnort
	$(OUIUSE) idx64srvroui.c
	$(PROMOTE) idx64srvr
idx64col.cc : base.tmc idx64.tmc /usr/local/share/huarp/flttime.tmc idx64col.tmc
	$(TMC.col) base.tmc idx64.tmc /usr/local/share/huarp/flttime.tmc idx64col.tmc
idx64coloui.c : $(OUIDIR)/collect.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/subbus.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/collect.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/subbus.oui $(OUIDIR)/msg.oui
idx64coloui.o : idx64coloui.c
idx64col.o : idx64col.cc
idx64tbl.tmc : idx64.tbl
	$(TABLE) idx64.tbl
idx64disp.cc : base.tmc idx64.tmc /usr/local/share/huarp/flttime.tmc idx64flag.tmc idx64tbl.tmc
	$(TMC) base.tmc idx64.tmc /usr/local/share/huarp/flttime.tmc idx64flag.tmc idx64tbl.tmc
idx64dispoui.c : $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
idx64dispoui.o : idx64dispoui.c
idx64disp.o : idx64disp.cc
idx64cmd.c : /usr/local/share/huarp/root.cmd /usr/local/share/huarp/idx64.cmd
	$(CMDGEN) -d idx64cmd.dat /usr/local/share/huarp/root.cmd /usr/local/share/huarp/idx64.cmd
idx64clt.o : idx64cmd.c
	$(COMPILE.clt) idx64cmd.c
idx64cltoui.c : $(OUIDIR)/kbdclt.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/kbdclt.oui $(OUIDIR)/msg.oui
idx64cltoui.o : idx64cltoui.c
idx64srvr.o : idx64cmd.c
	$(COMPILE.srvr) idx64cmd.c
idx64srvroui.c : $(OUIDIR)/cis.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/cis.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/msg.oui
idx64srvroui.o : idx64srvroui.c

tidy :
	rm -f $(OBJECT)
clean : tidy
	rm -f $(TARGET)
.cvsignore :
	@for i in Makefile '.B4*' $(OBJECT) $(TARGET); do echo "$$i"; done >.cvsignore
