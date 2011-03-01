# Makefile Generated Sun Feb 27 13:45:28 EST 2011
MNC=counters
HOMEDIR=/home/nort/BUILD/QNX6Build/QNX6/DACSdiag/counters
SPECFILE=counters.spec
SRC=counters.cmd base.tmc counts.tmc cnt_init.tmc counts.tbl
SRC+=counters.tma counters.doit
TOOL=counters.spec Makefile
SCRIPT=interact
OBJ=counterscmd.cc counterscmd.dat counterssrvr.o counterssrvroui.c
OBJ+=counterssrvroui.o counterscol.cc counters.pcm tm.dac
OBJ+=counterscoloui.c counterscoloui.o counterscol.o countstbl.tmc
OBJ+=countersdisp.cc countersdispoui.c countersdispoui.o
OBJ+=countersdisp.o countstblnc.tmc countersdispnc.cc
OBJ+=countersdispncoui.c countersdispncoui.o countersdispnc.o
OBJ+=countersalgoa.tmc countersalgo.cc countersalgooui.c
OBJ+=countersalgooui.o countersalgo.o doit.dep countersclt.o
OBJ+=counterscltoui.c counterscltoui.o counterscltnc.o
OBJ+=counterscltncoui.c counterscltncoui.o
TGT=counterssrvr counterscol countersdisp countersdispnc
TGT+=countersalgo doit countersclt counterscltnc
DISTRIB=counters.pcm tm.dac
IDISTRIB=Experiment.config VERSION doit
SRCDIST=counts.tbl counters.tma base.tmc
SRCDIST+=/usr/local/share/huarp/flttime.tmc counts.tmc
NONRCS=
TMCBASE=base.tmc /usr/local/share/huarp/flttime.tmc counts.tmc
CMDBASE=/usr/local/share/huarp/root.cmd
CMDBASE+=/usr/local/share/huarp/getcon.cmd counters.cmd
RCS=$(SRC) $(TOOL) $(SCRIPT)
SOURCE=$(RCS) $(NONRCS)
OBJECT=$(OBJ) *.err
TARGET=$(TGT)
DSTRB=$(TGT) $(DISTRIB) $(SCRIPT)
TGTDIR= $(TGTNODE)/home/DACS/counters
include /usr/local/share/huarp/appgen.mk

all : Makefile $(TGT)
Makefile : counters.spec
	appgen counters.spec
VERSION=`cat VERSION`
extdistribution : $(DSTRB) $(SRCDIST)
	@[ -d $(TGTDIR)/bin ] || mkdir $(TGTDIR)/bin
	@[ -d $(TGTDIR)/bin/$(VERSION) ] || mkdir $(TGTDIR)/bin/$(VERSION)
	@[ -d  $(TGTDIR)/bin/$(VERSION)/Base ] || mkdir $(TGTDIR)/bin/$(VERSION)/Base
	@distribute $(FORCE) $(TGTDIR)/bin/$(VERSION) $(DSTRB)
	@distribute $(FORCE) $(TGTDIR)/bin/$(VERSION)/Base $(SRCDIST)
distribution : extdistribution
	@distribute $(FORCE) $(TGTDIR) $(IDISTRIB)
counterssrvr : counterssrvr.o counterssrvroui.o
	$(LINK.privCXX) counterssrvr.o counterssrvroui.o -lsubbus -ltmpp -ltm -lnort
	$(OUIUSE) counterssrvroui.c
	$(PROMOTE) counterssrvr
counterscol : counterscol.o counterscoloui.o
	$(LINK.privCXX) counterscol.o counterscoloui.o -lsubbus -ltmpp -ltm -lnort
	$(OUIUSE) counterscoloui.c
	$(PROMOTE) counterscol
countersdisp : countersdisp.o countersdispoui.o
	$(LINK.normCXX) countersdisp.o countersdispoui.o -ldisp -ltmph -lph -ltmpp -ltm -lnort
	$(OUIUSE) countersdispoui.c
countersdispnc : countersdispnc.o countersdispncoui.o
	$(LINK.normCXX) countersdispnc.o countersdispncoui.o -ldispnc -ltmnc -lncurses -ltmpp -ltm -lnort
	$(OUIUSE) countersdispncoui.c
countersalgo : countersalgo.o countersalgooui.o
	$(LINK.normCXX) countersalgo.o countersalgooui.o -ltmpp -ltm -lnort
	$(OUIUSE) countersalgooui.c
doit : counters.doit
	mkdoit3 -o doit  counters.doit
countersclt : countersclt.o counterscltoui.o
	$(LINK.normCXX) countersclt.o counterscltoui.o -ltmph -lph -ltmpp -ltm -lnort
	$(OUIUSE) counterscltoui.c
counterscltnc : counterscltnc.o counterscltncoui.o
	$(LINK.normCXX) counterscltnc.o counterscltncoui.o -ltmnc -lncurses -ltmpp -ltm -lnort
	$(OUIUSE) counterscltncoui.c
counterscmd.cc : /usr/local/share/huarp/root.cmd /usr/local/share/huarp/getcon.cmd counters.cmd
	$(CMDGEN) -d counterscmd.dat /usr/local/share/huarp/root.cmd /usr/local/share/huarp/getcon.cmd counters.cmd
counterssrvr.o : counterscmd.cc
	$(COMPILE.srvr) counterscmd.cc
counterssrvroui.c : $(OUIDIR)/cis.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/subbus.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/cis.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/subbus.oui $(OUIDIR)/msg.oui
counterssrvroui.o : counterssrvroui.c
counterscol.cc : base.tmc /usr/local/share/huarp/flttime.tmc counts.tmc cnt_init.tmc
	$(TMC.col) base.tmc /usr/local/share/huarp/flttime.tmc counts.tmc cnt_init.tmc
counterscoloui.c : $(OUIDIR)/collect.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/subbus.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/collect.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/subbus.oui $(OUIDIR)/msg.oui
counterscoloui.o : counterscoloui.c
counterscol.o : counterscol.cc
countstbl.tmc : counts.tbl
	$(TABLE) counts.tbl
countersdisp.cc : base.tmc /usr/local/share/huarp/flttime.tmc counts.tmc countstbl.tmc
	$(TMC) base.tmc /usr/local/share/huarp/flttime.tmc counts.tmc countstbl.tmc
countersdispoui.c : $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
countersdispoui.o : countersdispoui.c
countersdisp.o : countersdisp.cc
countstblnc.tmc : counts.tbl
	$(NCTABLE) counts.tbl
countersdispnc.cc : base.tmc /usr/local/share/huarp/flttime.tmc counts.tmc countstblnc.tmc
	$(TMC) base.tmc /usr/local/share/huarp/flttime.tmc counts.tmc countstblnc.tmc
countersdispncoui.c : $(OUIDIR)/nctable.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/nctable.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
countersdispncoui.o : countersdispncoui.c
countersdispnc.o : countersdispnc.cc
countersalgoa.tmc : counters.tma counterssrvr
	$(SERVER) ./counterssrvr
	$(TMCALGO) -V "`./counterssrvr -R`" counters.tma
countersalgo.cc : base.tmc /usr/local/share/huarp/flttime.tmc counts.tmc countersalgoa.tmc
	$(TMC) base.tmc /usr/local/share/huarp/flttime.tmc counts.tmc countersalgoa.tmc
countersalgooui.c : $(OUIDIR)/tma.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/tma.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
countersalgooui.o : countersalgooui.c
countersalgo.o : countersalgo.cc
doit.dep : counters.doit
	mkdoit3 -M -o doit  counters.doit
-include doit.dep
countersclt.o : counterscmd.cc
	$(COMPILE.clt) counterscmd.cc
counterscltoui.c : $(OUIDIR)/kbdclt.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/kbdclt.oui $(OUIDIR)/msg.oui
counterscltoui.o : counterscltoui.c
counterscltnc.o : counterscmd.cc
	$(COMPILE.cltnc) counterscmd.cc
counterscltncoui.c : $(OUIDIR)/kbdclt.oui $(OUIDIR)/nctable.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/kbdclt.oui $(OUIDIR)/nctable.oui $(OUIDIR)/msg.oui
counterscltncoui.o : counterscltncoui.c

tidy :
	rm -f $(OBJECT)
clean : tidy
	rm -f $(TARGET)
.cvsignore :
	@for i in Makefile '.B4*' $(OBJECT) $(TARGET); do echo "$$i"; done >.cvsignore
