# Makefile Generated Tue Jul 17 13:01:21 EDT 2012
MNC=demo
HOMEDIR=/home/nort/BUILD/QNX6Build/QNX6/DASdemo
SPECFILE=demo.spec
SRC=types.tmc base.tmc demo.tbl demo.tma demo.doit
TOOL=demo.spec Makefile
SCRIPT=interact
OBJ=demotbl.tmc demodisp.cc demodispoui.c demodispoui.o demodisp.o
OBJ+=demotblnc.tmc demodispnc.cc demodispncoui.c demodispncoui.o
OBJ+=demodispnc.o demoalgoa.tmc demoalgo.cc demoalgooui.c
OBJ+=demoalgooui.o demoalgo.o doit.dep democmd.cc democmd.dat
OBJ+=democlt.o democltoui.c democltoui.o democltnc.o democltncoui.c
OBJ+=democltncoui.o demosrvr.o demosrvroui.c demosrvroui.o democol.cc
OBJ+=demo.pcm tm.dac democoloui.c democoloui.o democol.o
TGT=demodisp demodispnc demoalgo doit democlt democltnc demosrvr
TGT+=democol
DISTRIB=demo.pcm tm.dac
IDISTRIB=Experiment.config VERSION doit
SRCDIST=demo.tbl demo.tma types.tmc /usr/local/share/huarp/tmdf.tmc
SRCDIST+=/usr/local/share/huarp/cpu_usage.tmc
SRCDIST+=/usr/local/share/huarp/freemem.tmc
SRCDIST+=/usr/local/share/huarp/flttime.tmc base.tmc
NONRCS=
TMCBASE=types.tmc /usr/local/share/huarp/tmdf.tmc
TMCBASE+=/usr/local/share/huarp/cpu_usage.tmc
TMCBASE+=/usr/local/share/huarp/freemem.tmc
TMCBASE+=/usr/local/share/huarp/flttime.tmc base.tmc
CMDBASE=/usr/local/share/huarp/root.cmd
CMDBASE+=/usr/local/share/huarp/getcon.cmd
COLBASE=/usr/local/share/huarp/tmdf_col.tmc
COLBASE+=/usr/local/share/huarp/cpu_usage_col.tmc
COLBASE+=/usr/local/share/huarp/freemem_col.tmc
SWSBASE=
QCLIBASE=
PHTGT=demotbl.tmc
RCS=$(SRC) $(TOOL) $(SCRIPT)
SOURCE=$(RCS) $(NONRCS)
OBJECT=$(OBJ) *.err
TARGET=$(TGT)
DSTRB=$(TGT) $(DISTRIB) $(SCRIPT)
TGTDIR= $(TGTNODE)/home/DASdemo
include /usr/local/share/huarp/appgen.mk

.PHONY : all extdistribution distribution clean check_photon

all : Makefile check_photon $(TGT) .cvsignore
Makefile : demo.spec
	appgen demo.spec
VERSION=`cat VERSION`
extdistribution : $(DSTRB) $(SRCDIST) VERSION
	@[ -d $(TGTDIR)/bin ] || mkdir $(TGTDIR)/bin
	@[ -d $(TGTDIR)/bin/$(VERSION) ] || mkdir $(TGTDIR)/bin/$(VERSION)
	@[ -d  $(TGTDIR)/bin/$(VERSION)/Base ] || mkdir $(TGTDIR)/bin/$(VERSION)/Base
	@distribute $(FORCE) $(TGTDIR)/bin/$(VERSION) $(DSTRB)
	@distribute $(FORCE) $(TGTDIR)/bin/$(VERSION)/Base $(SRCDIST)
distribution : extdistribution
	@distribute $(FORCE) $(TGTDIR) $(IDISTRIB)
check_photon :
ifeq ($(WPHOTON),YES)
	@for tgt in $(PHTGT); do \
	  cmp -s $$tgt /usr/local/share/huarp/nulltbl.tmc && echo Remaking $$tgt && rm -f $$tgt; \
	done; true
endif
demodisp : demodisp.o demodispoui.o
	$(LINK.normCXX) demodisp.o demodispoui.o -ldisp -ltmph -lph -ltmpp -ltm -lnort
	$(OUIUSE) demodispoui.c
demodispnc : demodispnc.o demodispncoui.o
	$(LINK.normCXX) demodispnc.o demodispncoui.o -ldispnc -ltmnc -lncurses -ltmpp -ltm -lnort
	$(OUIUSE) demodispncoui.c
demoalgo : demoalgo.o demoalgooui.o
	$(LINK.normCXX) demoalgo.o demoalgooui.o -ltmpp -ltm -lnort
	$(OUIUSE) demoalgooui.c
doit : demo.doit
	mkdoit3 -o doit  demo.doit
democlt : democlt.o democltoui.o
	$(LINK.normCXX) democlt.o democltoui.o -ltmph -lph -ltmpp -ltm -lnort
	$(OUIUSE) democltoui.c
democltnc : democltnc.o democltncoui.o
	$(LINK.normCXX) democltnc.o democltncoui.o -ltmnc -lncurses -ltmpp -ltm -lnort
	$(OUIUSE) democltncoui.c
demosrvr : demosrvr.o demosrvroui.o
	$(LINK.privCXX) demosrvr.o demosrvroui.o -ltmpp -ltm -lnort
	$(OUIUSE) demosrvroui.c
	$(PROMOTE) demosrvr
democol : democol.o democoloui.o
	$(LINK.privCXX) democol.o democoloui.o -ltmpp -ltm -lnort
	$(OUIUSE) democoloui.c
	$(PROMOTE) democol
demotbl.tmc : demo.tbl
demodisp.cc : types.tmc /usr/local/share/huarp/tmdf.tmc /usr/local/share/huarp/cpu_usage.tmc /usr/local/share/huarp/freemem.tmc /usr/local/share/huarp/flttime.tmc base.tmc demotbl.tmc
	$(TMC) types.tmc /usr/local/share/huarp/tmdf.tmc /usr/local/share/huarp/cpu_usage.tmc /usr/local/share/huarp/freemem.tmc /usr/local/share/huarp/flttime.tmc base.tmc demotbl.tmc
demodispoui.c : $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
demodispoui.o : demodispoui.c
demodisp.o : demodisp.cc
demotblnc.tmc : demo.tbl
demodispnc.cc : types.tmc /usr/local/share/huarp/tmdf.tmc /usr/local/share/huarp/cpu_usage.tmc /usr/local/share/huarp/freemem.tmc /usr/local/share/huarp/flttime.tmc base.tmc demotblnc.tmc
	$(TMC) types.tmc /usr/local/share/huarp/tmdf.tmc /usr/local/share/huarp/cpu_usage.tmc /usr/local/share/huarp/freemem.tmc /usr/local/share/huarp/flttime.tmc base.tmc demotblnc.tmc
demodispncoui.c : $(OUIDIR)/nctable.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/nctable.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
demodispncoui.o : demodispncoui.c
demodispnc.o : demodispnc.cc
demoalgoa.tmc : demo.tma demosrvr
	$(SERVER) ./demosrvr
	$(TMCALGO) -V "`./demosrvr -R`" demo.tma
demoalgo.cc : types.tmc /usr/local/share/huarp/tmdf.tmc /usr/local/share/huarp/cpu_usage.tmc /usr/local/share/huarp/freemem.tmc /usr/local/share/huarp/flttime.tmc base.tmc demoalgoa.tmc
	$(TMC) types.tmc /usr/local/share/huarp/tmdf.tmc /usr/local/share/huarp/cpu_usage.tmc /usr/local/share/huarp/freemem.tmc /usr/local/share/huarp/flttime.tmc base.tmc demoalgoa.tmc
demoalgooui.c : $(OUIDIR)/tma.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/tma.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
demoalgooui.o : demoalgooui.c
demoalgo.o : demoalgo.cc
doit.dep : demo.doit
	mkdoit3 -M -o doit  demo.doit
-include doit.dep
democmd.cc : /usr/local/share/huarp/root.cmd /usr/local/share/huarp/getcon.cmd
	$(CMDGEN) -d democmd.dat /usr/local/share/huarp/root.cmd /usr/local/share/huarp/getcon.cmd
democlt.o : democmd.cc
	$(COMPILE.clt) democmd.cc
democltoui.c : $(OUIDIR)/kbdclt.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/kbdclt.oui $(OUIDIR)/msg.oui
democltoui.o : democltoui.c
democltnc.o : democmd.cc
	$(COMPILE.cltnc) democmd.cc
democltncoui.c : $(OUIDIR)/kbdclt.oui $(OUIDIR)/nctable.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/kbdclt.oui $(OUIDIR)/nctable.oui $(OUIDIR)/msg.oui
democltncoui.o : democltncoui.c
demosrvr.o : democmd.cc
	$(COMPILE.srvr) democmd.cc
demosrvroui.c : $(OUIDIR)/cis.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/cis.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/msg.oui
demosrvroui.o : demosrvroui.c
democol.cc : types.tmc /usr/local/share/huarp/tmdf.tmc /usr/local/share/huarp/cpu_usage.tmc /usr/local/share/huarp/freemem.tmc /usr/local/share/huarp/flttime.tmc base.tmc /usr/local/share/huarp/tmdf_col.tmc /usr/local/share/huarp/cpu_usage_col.tmc /usr/local/share/huarp/freemem_col.tmc
	$(TMC.col) types.tmc /usr/local/share/huarp/tmdf.tmc /usr/local/share/huarp/cpu_usage.tmc /usr/local/share/huarp/freemem.tmc /usr/local/share/huarp/flttime.tmc base.tmc /usr/local/share/huarp/tmdf_col.tmc /usr/local/share/huarp/cpu_usage_col.tmc /usr/local/share/huarp/freemem_col.tmc
democoloui.c : $(OUIDIR)/collect.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/collect.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/msg.oui
democoloui.o : democoloui.c
democol.o : democol.cc

tidy :
	rm -f $(OBJECT)
clean : tidy
	rm -f $(TARGET)
.cvsignore : Makefile
	@for i in $(OBJECT) $(TARGET); do echo "$$i"; done >.cvsignore
