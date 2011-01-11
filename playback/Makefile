# Makefile Generated Tue Jan 11 15:21:32 EST 2011
MNC=playback
HOMEDIR=/home/nort/BUILD/QNX6Build/QNX6/playback
SPECFILE=playback.spec
SRC=playback.cmd
TOOL=playback.spec Makefile
SCRIPT=
OBJ=playbackcmd.c playbackcmd.dat playbackclt.o playbackcltoui.c
OBJ+=playbackcltoui.o playbackcltnc.o playbackcltncoui.c
OBJ+=playbackcltncoui.o playbacksrvr.o playbacksrvroui.c
OBJ+=playbacksrvroui.o
TGT=playbackclt playbackcltnc playbacksrvr
DISTRIB=
IDISTRIB=Experiment.config VERSION
SRCDIST=
NONRCS=
TMCBASE=
CMDBASE=/usr/local/share/huarp/root.cmd playback.cmd
RCS=$(SRC) $(TOOL) $(SCRIPT)
SOURCE=$(RCS) $(NONRCS)
OBJECT=$(OBJ) *.err
TARGET=$(TGT)
DSTRB=$(TGT) $(DISTRIB) $(SCRIPT)
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
	$(LINK.norm) playbackclt.o playbackcltoui.o -ltmph -lph -ltm -lnort
	$(OUIUSE) playbackcltoui.c
playbackcltnc : playbackcltnc.o playbackcltncoui.o
	$(LINK.norm) playbackcltnc.o playbackcltncoui.o -ltmnc -lncurses -ltm -lnort
	$(OUIUSE) playbackcltncoui.c
playbacksrvr : playbacksrvr.o playbacksrvroui.o
	$(LINK.priv) playbacksrvr.o playbacksrvroui.o -ltm -lnort
	$(OUIUSE) playbacksrvroui.c
	$(PROMOTE) playbacksrvr
playbackcmd.c : /usr/local/share/huarp/root.cmd playback.cmd
	$(CMDGEN) -d playbackcmd.dat /usr/local/share/huarp/root.cmd playback.cmd
playbackclt.o : playbackcmd.c
	$(COMPILE.clt) playbackcmd.c
playbackcltoui.c : $(OUIDIR)/kbdclt.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/kbdclt.oui $(OUIDIR)/msg.oui
playbackcltoui.o : playbackcltoui.c
playbackcltnc.o : playbackcmd.c
	$(COMPILE.cltnc) playbackcmd.c
playbackcltncoui.c : $(OUIDIR)/kbdclt.oui $(OUIDIR)/nctable.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/kbdclt.oui $(OUIDIR)/nctable.oui $(OUIDIR)/msg.oui
playbackcltncoui.o : playbackcltncoui.c
playbacksrvr.o : playbackcmd.c
	$(COMPILE.srvr) playbackcmd.c
playbacksrvroui.c : $(OUIDIR)/cis.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/cis.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/msg.oui
playbacksrvroui.o : playbacksrvroui.c

tidy :
	rm -f $(OBJECT)
clean : tidy
	rm -f $(TARGET)
.cvsignore :
	@for i in Makefile '.B4*' $(OBJECT) $(TARGET); do echo "$$i"; done >.cvsignore
binPrograms = playbacksrvr playbackclt
install : $(binPrograms)
	cp $(binPrograms) $(TGTNODE)/usr/local/bin
