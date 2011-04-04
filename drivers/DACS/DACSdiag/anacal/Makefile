# Makefile Generated Thu Mar 24 16:52:18 EDT 2011
MNC=anacal
HOMEDIR=/home/nort/BUILD/QNX6Build/QNX6/DACSdiag/anacal
SPECFILE=anacal.spec
SRC=cmdenbl.cmd ana_in.cmd swstat.cmd ana_out.cmd anacal.tmc
SRC+=swstat.tmc ana_in_cfg.tmc anacal.tbl anacal.tma anacal.edf
SRC+=anacal.doit
TOOL=anacal.spec Makefile
SCRIPT=interact
OBJ=anacalcmd.cc anacalcmd.dat anacalsrvr.o anacalsrvroui.c
OBJ+=anacalsrvroui.o anacalcol.cc anacal.pcm tm.dac anacalcoloui.c
OBJ+=anacalcoloui.o anacalcol.o anacaltbl.tmc anacaldisp.cc
OBJ+=anacaldispoui.c anacaldispoui.o anacaldisp.o anacaltblnc.tmc
OBJ+=anacaldispnc.cc anacaldispncoui.c anacaldispncoui.o
OBJ+=anacaldispnc.o anacalalgoa.tmc anacalalgo.cc anacalalgooui.c
OBJ+=anacalalgooui.o anacalalgo.o anacalextedf.oui anacalext.tmc
OBJ+=anacalext.cc anacalextoui.c anacalextoui.o anacalext.o doit.dep
OBJ+=anacalclt.o anacalcltoui.c anacalcltoui.o anacalcltnc.o
OBJ+=anacalcltncoui.c anacalcltncoui.o
TGT=anacalsrvr anacalcol anacaldisp anacaldispnc anacalalgo
TGT+=anacalext doit anacalclt anacalcltnc
DISTRIB=anacal.pcm tm.dac
IDISTRIB=Experiment.config VERSION doit
SRCDIST=swstat.h anacal.tbl anacal.tma anacal.tmc
SRCDIST+=/usr/local/share/huarp/flttime.tmc swstat.tmc
NONRCS=
TMCBASE=anacal.tmc /usr/local/share/huarp/flttime.tmc swstat.tmc
CMDBASE=/usr/local/share/huarp/root.cmd
CMDBASE+=/usr/local/share/huarp/getcon.cmd cmdenbl.cmd ana_in.cmd
CMDBASE+=swstat.cmd ana_out.cmd
RCS=$(SRC) $(TOOL) $(SCRIPT)
SOURCE=$(RCS) $(NONRCS)
OBJECT=$(OBJ) *.err
TARGET=$(TGT)
DSTRB=$(TGT) $(DISTRIB) $(SCRIPT)
TGTDIR= $(TGTNODE)/home/DACS/anacal
include /usr/local/share/huarp/appgen.mk

all : Makefile $(TGT)
Makefile : anacal.spec
	appgen anacal.spec
VERSION=`cat VERSION`
extdistribution : $(DSTRB) $(SRCDIST)
	@[ -d $(TGTDIR)/bin ] || mkdir $(TGTDIR)/bin
	@[ -d $(TGTDIR)/bin/$(VERSION) ] || mkdir $(TGTDIR)/bin/$(VERSION)
	@[ -d  $(TGTDIR)/bin/$(VERSION)/Base ] || mkdir $(TGTDIR)/bin/$(VERSION)/Base
	@distribute $(FORCE) $(TGTDIR)/bin/$(VERSION) $(DSTRB)
	@distribute $(FORCE) $(TGTDIR)/bin/$(VERSION)/Base $(SRCDIST)
distribution : extdistribution
	@distribute $(FORCE) $(TGTDIR) $(IDISTRIB)
anacalsrvr : anacalsrvr.o anacalsrvroui.o
	$(LINK.privCXX) anacalsrvr.o anacalsrvroui.o -lsubbus -ltmpp -ltm -lnort
	$(OUIUSE) anacalsrvroui.c
	$(PROMOTE) anacalsrvr
anacalcol : anacalcol.o anacalcoloui.o
	$(LINK.privCXX) anacalcol.o anacalcoloui.o -lsubbus -ltmpp -ltm -lnort
	$(OUIUSE) anacalcoloui.c
	$(PROMOTE) anacalcol
anacaldisp : anacaldisp.o anacaldispoui.o
	$(LINK.normCXX) anacaldisp.o anacaldispoui.o -ldisp -ltmph -lph -ltmpp -ltm -lnort
	$(OUIUSE) anacaldispoui.c
anacaldispnc : anacaldispnc.o anacaldispncoui.o
	$(LINK.normCXX) anacaldispnc.o anacaldispncoui.o -ldispnc -ltmnc -lncurses -ltmpp -ltm -lnort
	$(OUIUSE) anacaldispncoui.c
anacalalgo : anacalalgo.o anacalalgooui.o
	$(LINK.normCXX) anacalalgo.o anacalalgooui.o -ltmpp -ltm -lnort
	$(OUIUSE) anacalalgooui.c
anacalext : anacalext.o anacalextoui.o
	$(LINK.normCXX) anacalext.o anacalextoui.o -ltmpp -ltm -lssp -lmem -lm -lnort
	$(OUIUSE) anacalextoui.c
doit : anacal.doit
	mkdoit3 -o doit  anacal.doit
anacalclt : anacalclt.o anacalcltoui.o
	$(LINK.normCXX) anacalclt.o anacalcltoui.o -ltmph -lph -ltmpp -ltm -lnort
	$(OUIUSE) anacalcltoui.c
anacalcltnc : anacalcltnc.o anacalcltncoui.o
	$(LINK.normCXX) anacalcltnc.o anacalcltncoui.o -ltmnc -lncurses -ltmpp -ltm -lnort
	$(OUIUSE) anacalcltncoui.c
anacalcmd.cc : /usr/local/share/huarp/root.cmd /usr/local/share/huarp/getcon.cmd cmdenbl.cmd ana_in.cmd swstat.cmd ana_out.cmd
	$(CMDGEN) -d anacalcmd.dat /usr/local/share/huarp/root.cmd /usr/local/share/huarp/getcon.cmd cmdenbl.cmd ana_in.cmd swstat.cmd ana_out.cmd
anacalsrvr.o : anacalcmd.cc
	$(COMPILE.srvr) anacalcmd.cc
anacalsrvroui.c : $(OUIDIR)/cis.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/subbus.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/cis.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/subbus.oui $(OUIDIR)/msg.oui
anacalsrvroui.o : anacalsrvroui.c
anacalcol.cc : anacal.tmc /usr/local/share/huarp/flttime.tmc swstat.tmc ana_in_cfg.tmc
	$(TMC.col) anacal.tmc /usr/local/share/huarp/flttime.tmc swstat.tmc ana_in_cfg.tmc
anacalcoloui.c : $(OUIDIR)/collect.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/subbus.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/collect.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/subbus.oui $(OUIDIR)/msg.oui
anacalcoloui.o : anacalcoloui.c
anacalcol.o : anacalcol.cc
anacaltbl.tmc : anacal.tbl
	$(TABLE) anacal.tbl
anacaldisp.cc : anacal.tmc /usr/local/share/huarp/flttime.tmc swstat.tmc anacaltbl.tmc
	$(TMC) anacal.tmc /usr/local/share/huarp/flttime.tmc swstat.tmc anacaltbl.tmc
anacaldispoui.c : $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
anacaldispoui.o : anacaldispoui.c
anacaldisp.o : anacaldisp.cc
anacaltblnc.tmc : anacal.tbl
	$(NCTABLE) anacal.tbl
anacaldispnc.cc : anacal.tmc /usr/local/share/huarp/flttime.tmc swstat.tmc anacaltblnc.tmc
	$(TMC) anacal.tmc /usr/local/share/huarp/flttime.tmc swstat.tmc anacaltblnc.tmc
anacaldispncoui.c : $(OUIDIR)/nctable.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/nctable.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
anacaldispncoui.o : anacaldispncoui.c
anacaldispnc.o : anacaldispnc.cc
anacalalgoa.tmc : anacal.tma anacalsrvr
	$(SERVER) ./anacalsrvr
	$(TMCALGO) -V "`./anacalsrvr -R`" anacal.tma
anacalalgo.cc : anacal.tmc /usr/local/share/huarp/flttime.tmc swstat.tmc anacalalgoa.tmc
	$(TMC) anacal.tmc /usr/local/share/huarp/flttime.tmc swstat.tmc anacalalgoa.tmc
anacalalgooui.c : $(OUIDIR)/tma.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/tma.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
anacalalgooui.o : anacalalgooui.c
anacalalgo.o : anacalalgo.cc
anacalextedf.oui : anacal.edf
	$(EDF2OUI) anacal.edf
anacalext.tmc : anacal.edf
	$(EDF2EXT) anacal.edf
anacalext.cc : anacal.tmc /usr/local/share/huarp/flttime.tmc swstat.tmc anacalext.tmc
	$(TMC) anacal.tmc /usr/local/share/huarp/flttime.tmc swstat.tmc anacalext.tmc
anacalextoui.c : anacalextedf.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) anacalextedf.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
anacalextoui.o : anacalextoui.c
anacalext.o : anacalext.cc
doit.dep : anacal.doit
	mkdoit3 -M -o doit  anacal.doit
-include doit.dep
anacalclt.o : anacalcmd.cc
	$(COMPILE.clt) anacalcmd.cc
anacalcltoui.c : $(OUIDIR)/kbdclt.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/kbdclt.oui $(OUIDIR)/msg.oui
anacalcltoui.o : anacalcltoui.c
anacalcltnc.o : anacalcmd.cc
	$(COMPILE.cltnc) anacalcmd.cc
anacalcltncoui.c : $(OUIDIR)/kbdclt.oui $(OUIDIR)/nctable.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/kbdclt.oui $(OUIDIR)/nctable.oui $(OUIDIR)/msg.oui
anacalcltncoui.o : anacalcltncoui.c

tidy :
	rm -f $(OBJECT)
clean : tidy
	rm -f $(TARGET)
.cvsignore :
	@for i in Makefile '.B4*' $(OBJECT) $(TARGET); do echo "$$i"; done >.cvsignore
