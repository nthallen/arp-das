# Makefile Generated Tue Mar 01 11:14:18 EST 2011
MNC=playback
HOMEDIR=/home/nort/BUILD/QNX6Build/QNX6/playback
SPECFILE=playback.spec
SRC=playback.cmd
TOOL=playback.spec Makefile
SCRIPT=
OBJ=FullBuild.* playbackcmd.cc playbackcmd.dat playbackclt.o
OBJ+=playbackcltoui.c playbackcltoui.o playbackcltnc.o
OBJ+=playbackcltncoui.c playbackcltncoui.o playbacksrvr.o
OBJ+=playbacksrvroui.c playbacksrvroui.o
TGT=playbackclt playbackcltnc playbacksrvr
DISTRIB=
IDISTRIB=Experiment.config VERSION
SRCDIST=
NONRCS=
TMCBASE=
CMDBASE=/usr/local/share/huarp/root.cmd
CMDBASE+=/usr/local/share/huarp/getcon.cmd playback.cmd
RCS=$(SRC) $(TOOL) $(SCRIPT)
SOURCE=$(RCS) $(NONRCS)
OBJECT=$(OBJ) *.err
TARGET=$(TGT)
DSTRB=$(TGT) $(DISTRIB) $(SCRIPT)
TGTDIR= $(PWD)
include /usr/local/share/huarp/appgen.mk

all : Makefile $(TGT)
Makefile : playback.spec
	appgen playback.spec
VERSION=`cat VERSION`
extdistribution : $(DSTRB) $(SRCDIST)
	@[ -d $(TGTDIR)/bin ] || mkdir $(TGTDIR)/bin
	@[ -d $(TGTDIR)/bin/$(VERSION) ] || mkdir $(TGTDIR)/bin/$(VERSION)
	@[ -d  $(TGTDIR)/bin/$(VERSION)/Base ] || mkdir $(TGTDIR)/bin/$(VERSION)/Base
	@distribute $(FORCE) $(TGTDIR)/bin/$(VERSION) $(DSTRB)
	@distribute $(FORCE) $(TGTDIR)/bin/$(VERSION)/Base $(SRCDIST)
distribution : extdistribution
	@distribute $(FORCE) $(TGTDIR) $(IDISTRIB)
playbackclt : playbackclt.o playbackcltoui.o
	$(LINK.normCXX) playbackclt.o playbackcltoui.o -ltmph -lph -ltmpp -ltm -lnort
	$(OUIUSE) playbackcltoui.c
playbackcltnc : playbackcltnc.o playbackcltncoui.o
	$(LINK.normCXX) playbackcltnc.o playbackcltncoui.o -ltmnc -lncurses -ltmpp -ltm -lnort
	$(OUIUSE) playbackcltncoui.c
playbacksrvr : playbacksrvr.o playbacksrvroui.o
	$(LINK.privCXX) playbacksrvr.o playbacksrvroui.o -ltmpp -ltm -lnort
	$(OUIUSE) playbacksrvroui.c
	$(PROMOTE) playbacksrvr
playbackcmd.cc : /usr/local/share/huarp/root.cmd /usr/local/share/huarp/getcon.cmd playback.cmd
	$(CMDGEN) -d playbackcmd.dat /usr/local/share/huarp/root.cmd /usr/local/share/huarp/getcon.cmd playback.cmd
playbackclt.o : playbackcmd.cc
	$(COMPILE.clt) playbackcmd.cc
playbackcltoui.c : $(OUIDIR)/kbdclt.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/kbdclt.oui $(OUIDIR)/msg.oui
playbackcltoui.o : playbackcltoui.c
playbackcltnc.o : playbackcmd.cc
	$(COMPILE.cltnc) playbackcmd.cc
playbackcltncoui.c : $(OUIDIR)/kbdclt.oui $(OUIDIR)/nctable.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/kbdclt.oui $(OUIDIR)/nctable.oui $(OUIDIR)/msg.oui
playbackcltncoui.o : playbackcltncoui.c
playbacksrvr.o : playbackcmd.cc
	$(COMPILE.srvr) playbackcmd.cc
playbacksrvroui.c : $(OUIDIR)/cis.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/cis.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/msg.oui
playbacksrvroui.o : playbacksrvroui.c

tidy :
	rm -f $(OBJECT)
clean : tidy
	rm -f $(TARGET)
.cvsignore :
	@for i in Makefile '.B4*' $(OBJECT) $(TARGET); do echo "$$i"; done >.cvsignore
binPrograms = playbacksrvr playbackclt playbackcltnc
install : $(binPrograms)
	cp -pnv $(binPrograms) $(TGTNODE)/usr/local/bin
