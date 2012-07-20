# Makefile Generated Fri Jul 20 14:01:22 EDT 2012
MNC=demo
HOMEDIR=/home/nort/BUILD/QNX6Build/QNX6/DASdemo
SPECFILE=demo.spec
SRC=types.tmc base.tmc SWData.tmc SWData.h demo.tbl demo.tma
SRC+=demo.sws demo.doit SWData.cmd SWData_col.tmc
TOOL=demo.spec Makefile
SCRIPT=interact
OBJ=SWData.cmd SWData.h SWData.tmc SWData_col.tmc demotbl.tmc
OBJ+=demodisp.cc demodispoui.c demodispoui.o demodisp.o demotblnc.tmc
OBJ+=demodispnc.cc demodispncoui.c demodispncoui.o demodispnc.o
OBJ+=demosws.tma demoalgoa.tmc demoalgo.cc demoalgooui.c
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
SRCDIST+=/usr/local/share/huarp/flttime.tmc base.tmc SWData.tmc
SRCDIST+=SWData.h
NONRCS=
TMCBASE=types.tmc /usr/local/share/huarp/tmdf.tmc
TMCBASE+=/usr/local/share/huarp/cpu_usage.tmc
TMCBASE+=/usr/local/share/huarp/freemem.tmc
TMCBASE+=/usr/local/share/huarp/flttime.tmc base.tmc SWData.tmc
TMCBASE+=SWData.h
CMDBASE=/usr/local/share/huarp/root.cmd
CMDBASE+=/usr/local/share/huarp/getcon.cmd SWData.cmd
COLBASE=/usr/local/share/huarp/tmdf_col.tmc
COLBASE+=/usr/local/share/huarp/cpu_usage_col.tmc
COLBASE+=/usr/local/share/huarp/freemem_col.tmc SWData_col.tmc
SWSBASE=demo.sws
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
demodisp.cc : types.tmc /usr/local/share/huarp/tmdf.tmc /usr/local/share/huarp/cpu_usage.tmc /usr/local/share/huarp/freemem.tmc /usr/local/share/huarp/flttime.tmc base.tmc SWData.tmc demotbl.tmc
	$(TMC) types.tmc /usr/local/share/huarp/tmdf.tmc /usr/local/share/huarp/cpu_usage.tmc /usr/local/share/huarp/freemem.tmc /usr/local/share/huarp/flttime.tmc base.tmc SWData.tmc demotbl.tmc
demodispoui.c : $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
demodispoui.o : demodispoui.c SWData.h
demodisp.o : demodisp.cc SWData.h
demotblnc.tmc : demo.tbl
demodispnc.cc : types.tmc /usr/local/share/huarp/tmdf.tmc /usr/local/share/huarp/cpu_usage.tmc /usr/local/share/huarp/freemem.tmc /usr/local/share/huarp/flttime.tmc base.tmc SWData.tmc demotblnc.tmc
	$(TMC) types.tmc /usr/local/share/huarp/tmdf.tmc /usr/local/share/huarp/cpu_usage.tmc /usr/local/share/huarp/freemem.tmc /usr/local/share/huarp/flttime.tmc base.tmc SWData.tmc demotblnc.tmc
demodispncoui.c : $(OUIDIR)/nctable.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/nctable.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
demodispncoui.o : demodispncoui.c SWData.h
demodispnc.o : demodispnc.cc SWData.h
demosws.tma : demo.sws
	$(COMPILE.sws) demo.sws
demoalgoa.tmc : demo.tma demosws.tma demosrvr
	$(SERVER) ./demosrvr
	$(TMCALGO) -V "`./demosrvr -R`" demo.tma demosws.tma
demoalgo.cc : types.tmc /usr/local/share/huarp/tmdf.tmc /usr/local/share/huarp/cpu_usage.tmc /usr/local/share/huarp/freemem.tmc /usr/local/share/huarp/flttime.tmc base.tmc SWData.tmc demoalgoa.tmc
	$(TMC) types.tmc /usr/local/share/huarp/tmdf.tmc /usr/local/share/huarp/cpu_usage.tmc /usr/local/share/huarp/freemem.tmc /usr/local/share/huarp/flttime.tmc base.tmc SWData.tmc demoalgoa.tmc
demoalgooui.c : $(OUIDIR)/tma.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/tma.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
demoalgooui.o : demoalgooui.c SWData.h
demoalgo.o : demoalgo.cc SWData.h
doit.dep : demo.doit
	mkdoit3 -M -o doit  demo.doit
-include doit.dep
democmd.cc : /usr/local/share/huarp/root.cmd /usr/local/share/huarp/getcon.cmd SWData.cmd
	$(CMDGEN) -d democmd.dat /usr/local/share/huarp/root.cmd /usr/local/share/huarp/getcon.cmd SWData.cmd
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
democol.cc : types.tmc /usr/local/share/huarp/tmdf.tmc /usr/local/share/huarp/cpu_usage.tmc /usr/local/share/huarp/freemem.tmc /usr/local/share/huarp/flttime.tmc base.tmc SWData.tmc /usr/local/share/huarp/tmdf_col.tmc /usr/local/share/huarp/cpu_usage_col.tmc /usr/local/share/huarp/freemem_col.tmc SWData_col.tmc
	$(TMC.col) types.tmc /usr/local/share/huarp/tmdf.tmc /usr/local/share/huarp/cpu_usage.tmc /usr/local/share/huarp/freemem.tmc /usr/local/share/huarp/flttime.tmc base.tmc SWData.tmc /usr/local/share/huarp/tmdf_col.tmc /usr/local/share/huarp/cpu_usage_col.tmc /usr/local/share/huarp/freemem_col.tmc SWData_col.tmc
democoloui.c : $(OUIDIR)/collect.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/collect.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/msg.oui
democoloui.o : democoloui.c SWData.h
democol.o : democol.cc SWData.h
SWData.h SWData.tmc SWData_col.tmc SWData.cmd :  demo.sws
	swscomp -o SWData  demo.sws

tidy :
	rm -f $(OBJECT)
clean : tidy
	rm -f $(TARGET)
.cvsignore : Makefile
	@for i in $(OBJECT) $(TARGET); do echo "$$i"; done >.cvsignore
