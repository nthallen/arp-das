# $Log$
# Revision 1.2  2008/07/23 13:07:42  nort
# Uncommitted changes and imports
#
# Revision 1.1  2001/07/24 18:50:33  nort
# Initial revision
#
SRC=digital2.c bckpln.c dtoatest.c o3ctdiag.c failtest.c tmrtest.c
SRC+=indxdiag.c scdebug.c bckpln.oui bckpln.h
SRC+=ugctdiag.c gtctdiag.c h2octdiag.c gtct104.c
SRC+=sbswitch.c
OBJ=digital2.o bckpln.o bckpln_pi.o dtoatest.o o3ctdiag.o tmrtest.o
OBJ+=indxdiag.o scdebug.o bckplnoui.o bckplnoui.c failtest.o
OBJ+=ugctdiag.o gtctdiag.o h2octdiag.o gtct104.o sbswitch.o
TEMP=*.err
TOOL=Makefile

HOMEDIR=/usr/local/src/das/diagnost
MNC=diag
SOURCE=$(SRC) $(TOOL)
OBJECT=$(OBJ) $(TEMP)
TARGET=digital2 bckpln dtoatest o3ctdiag failtest tmrtest indxdiag scdebug
TARGET+=ugctdiag gtctdiag h2octdiag gtct104 sbswitch
SUBDIRS=lib diagl indexer idx

LLIB=/usr/local/lib
CFLAGS=-ms -2 -g -w4
LDFLAGS=-b -l diaglib
LINK.priv=$(LINK.c) $(LDFLAGS) -T 1 -o $@
LINK.norm=$(LINK.c) $(LDFLAGS) -o $@
LINK.ext=$(LINK.c) $(LDFLAGS) -o $@

all : $(TARGET)

bckpln : bckpln.o bckplnoui.o
	$(LINK.priv) -l curses bckpln.o bckplnoui.o
	# ln -f bckpln /usr/local/bin/bckpln
	usemsg bckpln bckplnoui.c
	promote bckpln
bckplnoui.o : bckplnoui.c bckpln.h
bckplnoui.c : bckpln.oui
	oui -o bckplnoui.c bckpln.oui
bckpln_pi : bckpln_pi.o
	$(LINK.priv) -l curses bckpln_pi.o
	ln -f bckpln_pi /usr/local/bin/bckpln_pi
	promote bckpln_pi
digital2 : digital2.o $(LLIB)/subbuss.lib
	$(LINK.priv) digital2.o
	ln -f digital2 /usr/local/bin/digital2
	promote digital2
dtoatest : dtoatest.o $(LLIB)/subbuss.lib $(LLIB)/diaglibs.lib
	$(LINK.priv) dtoatest.o
	ln -f dtoatest /usr/local/bin/dtoatest
	promote dtoatest
o3ctdiag : o3ctdiag.o $(LLIB)/subbuss.lib $(LLIB)/diaglibs.lib
	$(LINK.priv) o3ctdiag
	usemsg o3ctdiag o3ctdiag.c
	ln -f o3ctdiag /usr/local/bin/o3ctdiag
	promote o3ctdiag
h2octdiag : h2octdiag.o $(LLIB)/subbuss.lib $(LLIB)/diaglibs.lib
	$(LINK.priv) h2octdiag
	usemsg h2octdiag h2octdiag.c
	# ln -f h2octdiag /usr/local/bin/h2octdiag
	promote h2octdiag
bckpln_pi.o : bckpln.c
	$(COMPILE.c) -fo$@ -DPCICC=1 bckpln.c
tmrtest : tmrtest.o
	$(LINK.norm) tmrtest -l eillib
tmrtest.o : tmrtest.c

indxdiag : indxdiag.o
	$(LINK.priv) indxdiag -l diaglib
	promote indxdiag
indxdiag.o : indxdiag.c
scdebug : scdebug.o $(LLIB)/diaglibs.lib
	$(LINK.priv) scdebug
	usemsg scdebug scdebug.c
	ln -f scdebug /usr/local/bin/scdebug
	promote scdebug
scdebug.o : scdebug.c
failtest : failtest.o
	rm -f failtest
	$(LINK.priv) failtest -l diaglib
	usemsg failtest failtest.c
	promote failtest
failtest.o : failtest.c
ugctdiag : ugctdiag.o
	$(LINK.priv) ugctdiag
	promote ugctdiag
ugctdiag.o : ugctdiag.c
gtctdiag : gtctdiag.o
	$(LINK.priv) gtctdiag
	promote gtctdiag
gtctdiag.o : gtctdiag.c
gtct104 : gtct104.o
	$(LINK.priv) gtct104
	usemsg gtct104 gtct104.c
	promote gtct104

sbswitch.o : sbswitch.c
sbswitch : sbswitch.o
	rm -f sbswitch
	$(LINK.priv) sbswitch
	usemsg sbswitch sbswitch.c
	promote sbswitch
	