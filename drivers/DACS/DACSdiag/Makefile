# Makefile Generated Wed Apr 06 22:20:40 CDT 2011
MNC=DACS
HOMEDIR=/home/nort/BUILD/QNX6Build/QNX6/DACSdiag
SPECFILE=DACS.spec
SRC=cmdenbl.cmd ana_in.cmd idx64drv.cmd swstat.cmd dccc.cmd
SRC+=ana_out.cmd DACS.tmc idx64.tmc swstat.tmc ana_in.tmc ana_out.tmc
SRC+=counts.tmc idx64col.tmc ana_in_cfg.tmc cnt_init.tmc
SRC+=idx64flag.tmc idx64.tbl ana_in.tbl digio.tmc digio.tbl
SRC+=ana_out.tbl counts.tbl DACS.tma DACS.doit
TOOL=DACS.spec Makefile
SCRIPT=idx64.idx64 interact dccc.dccc
OBJ=DACScmd.cc DACScmd.dat DACSsrvr.o DACSsrvroui.c DACSsrvroui.o
OBJ+=DACScol.cc DACS.pcm tm.dac DACScoloui.c DACScoloui.o DACScol.o
OBJ+=idx64tbl.tmc ana_intbl.tmc digiotbl.tmc ana_outtbl.tmc
OBJ+=countstbl.tmc DACSdisp.cc DACSdispoui.c DACSdispoui.o DACSdisp.o
OBJ+=idx64tblnc.tmc ana_intblnc.tmc digiotblnc.tmc ana_outtblnc.tmc
OBJ+=countstblnc.tmc DACSdispnc.cc DACSdispncoui.c DACSdispncoui.o
OBJ+=DACSdispnc.o DACSalgoa.tmc DACSalgo.cc DACSalgooui.c
OBJ+=DACSalgooui.o DACSalgo.o doit.dep DACSclt.o DACScltoui.c
OBJ+=DACScltoui.o DACScltnc.o DACScltncoui.c DACScltncoui.o
TGT=DACSsrvr DACScol DACSdisp DACSdispnc DACSalgo doit DACSclt
TGT+=DACScltnc
DISTRIB=DACS.pcm tm.dac
IDISTRIB=Experiment.config VERSION doit
SRCDIST=swstat.h idx64.tbl ana_in.tbl digio.tbl ana_out.tbl
SRCDIST+=counts.tbl DACS.tma DACS.tmc idx64.tmc
SRCDIST+=/usr/local/share/huarp/flttime.tmc swstat.tmc ana_in.tmc
SRCDIST+=ana_out.tmc counts.tmc
NONRCS=
TMCBASE=DACS.tmc idx64.tmc /usr/local/share/huarp/flttime.tmc
TMCBASE+=swstat.tmc ana_in.tmc ana_out.tmc counts.tmc
CMDBASE=/usr/local/share/huarp/root.cmd
CMDBASE+=/usr/local/share/huarp/getcon.cmd
CMDBASE+=/usr/local/share/huarp/idx64.cmd cmdenbl.cmd ana_in.cmd
CMDBASE+=idx64drv.cmd swstat.cmd dccc.cmd ana_out.cmd
RCS=$(SRC) $(TOOL) $(SCRIPT)
SOURCE=$(RCS) $(NONRCS)
OBJECT=$(OBJ) *.err
TARGET=$(TGT)
DSTRB=$(TGT) $(DISTRIB) $(SCRIPT)
TGTDIR= $(TGTNODE)/home/DACS
include /usr/local/share/huarp/appgen.mk

all : Makefile $(TGT)
Makefile : DACS.spec
	appgen DACS.spec
VERSION=`cat VERSION`
extdistribution : $(DSTRB) $(SRCDIST)
	@[ -d $(TGTDIR)/bin ] || mkdir $(TGTDIR)/bin
	@[ -d $(TGTDIR)/bin/$(VERSION) ] || mkdir $(TGTDIR)/bin/$(VERSION)
	@[ -d  $(TGTDIR)/bin/$(VERSION)/Base ] || mkdir $(TGTDIR)/bin/$(VERSION)/Base
	@distribute $(FORCE) $(TGTDIR)/bin/$(VERSION) $(DSTRB)
	@distribute $(FORCE) $(TGTDIR)/bin/$(VERSION)/Base $(SRCDIST)
distribution : extdistribution
	@distribute $(FORCE) $(TGTDIR) $(IDISTRIB)
DACSsrvr : DACSsrvr.o DACSsrvroui.o
	$(LINK.privCXX) DACSsrvr.o DACSsrvroui.o -lsubbus -ltmpp -ltm -lnort
	$(OUIUSE) DACSsrvroui.c
	$(PROMOTE) DACSsrvr
DACScol : DACScol.o DACScoloui.o
	$(LINK.privCXX) DACScol.o DACScoloui.o -lsubbus -ltmpp -ltm -lnort
	$(OUIUSE) DACScoloui.c
	$(PROMOTE) DACScol
DACSdisp : DACSdisp.o DACSdispoui.o
	$(LINK.normCXX) DACSdisp.o DACSdispoui.o -ldisp -ltmph -lph -ltmpp -ltm -lnort
	$(OUIUSE) DACSdispoui.c
DACSdispnc : DACSdispnc.o DACSdispncoui.o
	$(LINK.normCXX) DACSdispnc.o DACSdispncoui.o -ldispnc -ltmnc -lncurses -ltmpp -ltm -lnort
	$(OUIUSE) DACSdispncoui.c
DACSalgo : DACSalgo.o DACSalgooui.o
	$(LINK.normCXX) DACSalgo.o DACSalgooui.o -ltmpp -ltm -lnort
	$(OUIUSE) DACSalgooui.c
doit : DACS.doit
	mkdoit3 -o doit  DACS.doit
DACSclt : DACSclt.o DACScltoui.o
	$(LINK.normCXX) DACSclt.o DACScltoui.o -ltmph -lph -ltmpp -ltm -lnort
	$(OUIUSE) DACScltoui.c
DACScltnc : DACScltnc.o DACScltncoui.o
	$(LINK.normCXX) DACScltnc.o DACScltncoui.o -ltmnc -lncurses -ltmpp -ltm -lnort
	$(OUIUSE) DACScltncoui.c
DACScmd.cc : /usr/local/share/huarp/root.cmd /usr/local/share/huarp/getcon.cmd /usr/local/share/huarp/idx64.cmd cmdenbl.cmd ana_in.cmd idx64drv.cmd swstat.cmd dccc.cmd ana_out.cmd
	$(CMDGEN) -d DACScmd.dat /usr/local/share/huarp/root.cmd /usr/local/share/huarp/getcon.cmd /usr/local/share/huarp/idx64.cmd cmdenbl.cmd ana_in.cmd idx64drv.cmd swstat.cmd dccc.cmd ana_out.cmd
DACSsrvr.o : DACScmd.cc
	$(COMPILE.srvr) DACScmd.cc
DACSsrvroui.c : $(OUIDIR)/cis.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/subbus.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/cis.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/subbus.oui $(OUIDIR)/msg.oui
DACSsrvroui.o : DACSsrvroui.c
DACScol.cc : DACS.tmc idx64.tmc /usr/local/share/huarp/flttime.tmc swstat.tmc ana_in.tmc ana_out.tmc counts.tmc idx64col.tmc ana_in_cfg.tmc /usr/local/share/huarp/DACS_ID.tmc cnt_init.tmc
	$(TMC.col) DACS.tmc idx64.tmc /usr/local/share/huarp/flttime.tmc swstat.tmc ana_in.tmc ana_out.tmc counts.tmc idx64col.tmc ana_in_cfg.tmc /usr/local/share/huarp/DACS_ID.tmc cnt_init.tmc
DACScoloui.c : $(OUIDIR)/collect.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/subbus.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/collect.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/subbus.oui $(OUIDIR)/msg.oui
DACScoloui.o : DACScoloui.c
DACScol.o : DACScol.cc
idx64tbl.tmc : idx64.tbl
	$(TABLE) idx64.tbl
ana_intbl.tmc : ana_in.tbl
	$(TABLE) ana_in.tbl
digiotbl.tmc : digio.tbl
	$(TABLE) digio.tbl
ana_outtbl.tmc : ana_out.tbl
	$(TABLE) ana_out.tbl
countstbl.tmc : counts.tbl
	$(TABLE) counts.tbl
DACSdisp.cc : DACS.tmc idx64.tmc /usr/local/share/huarp/flttime.tmc swstat.tmc ana_in.tmc ana_out.tmc counts.tmc idx64flag.tmc idx64tbl.tmc ana_intbl.tmc digio.tmc digiotbl.tmc ana_outtbl.tmc countstbl.tmc
	$(TMC) DACS.tmc idx64.tmc /usr/local/share/huarp/flttime.tmc swstat.tmc ana_in.tmc ana_out.tmc counts.tmc idx64flag.tmc idx64tbl.tmc ana_intbl.tmc digio.tmc digiotbl.tmc ana_outtbl.tmc countstbl.tmc
DACSdispoui.c : $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
DACSdispoui.o : DACSdispoui.c
DACSdisp.o : DACSdisp.cc
idx64tblnc.tmc : idx64.tbl
	$(NCTABLE) idx64.tbl
ana_intblnc.tmc : ana_in.tbl
	$(NCTABLE) ana_in.tbl
digiotblnc.tmc : digio.tbl
	$(NCTABLE) digio.tbl
ana_outtblnc.tmc : ana_out.tbl
	$(NCTABLE) ana_out.tbl
countstblnc.tmc : counts.tbl
	$(NCTABLE) counts.tbl
DACSdispnc.cc : DACS.tmc idx64.tmc /usr/local/share/huarp/flttime.tmc swstat.tmc ana_in.tmc ana_out.tmc counts.tmc idx64flag.tmc idx64tblnc.tmc ana_intblnc.tmc digio.tmc digiotblnc.tmc ana_outtblnc.tmc countstblnc.tmc
	$(TMC) DACS.tmc idx64.tmc /usr/local/share/huarp/flttime.tmc swstat.tmc ana_in.tmc ana_out.tmc counts.tmc idx64flag.tmc idx64tblnc.tmc ana_intblnc.tmc digio.tmc digiotblnc.tmc ana_outtblnc.tmc countstblnc.tmc
DACSdispncoui.c : $(OUIDIR)/nctable.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/nctable.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
DACSdispncoui.o : DACSdispncoui.c
DACSdispnc.o : DACSdispnc.cc
DACSalgoa.tmc : DACS.tma DACSsrvr
	$(SERVER) ./DACSsrvr
	$(TMCALGO) -V "`./DACSsrvr -R`" DACS.tma
DACSalgo.cc : DACS.tmc idx64.tmc /usr/local/share/huarp/flttime.tmc swstat.tmc ana_in.tmc ana_out.tmc counts.tmc DACSalgoa.tmc
	$(TMC) DACS.tmc idx64.tmc /usr/local/share/huarp/flttime.tmc swstat.tmc ana_in.tmc ana_out.tmc counts.tmc DACSalgoa.tmc
DACSalgooui.c : $(OUIDIR)/tma.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/tma.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
DACSalgooui.o : DACSalgooui.c
DACSalgo.o : DACSalgo.cc
doit.dep : DACS.doit
	mkdoit3 -M -o doit  DACS.doit
-include doit.dep
DACSclt.o : DACScmd.cc
	$(COMPILE.clt) DACScmd.cc
DACScltoui.c : $(OUIDIR)/kbdclt.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/kbdclt.oui $(OUIDIR)/msg.oui
DACScltoui.o : DACScltoui.c
DACScltnc.o : DACScmd.cc
	$(COMPILE.cltnc) DACScmd.cc
DACScltncoui.c : $(OUIDIR)/kbdclt.oui $(OUIDIR)/nctable.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/kbdclt.oui $(OUIDIR)/nctable.oui $(OUIDIR)/msg.oui
DACScltncoui.o : DACScltncoui.c

tidy :
	rm -f $(OBJECT)
clean : tidy
	rm -f $(TARGET)
.cvsignore :
	@for i in Makefile '.B4*' $(OBJECT) $(TARGET); do echo "$$i"; done >.cvsignore
