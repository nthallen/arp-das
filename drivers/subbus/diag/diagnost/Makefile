# $Log$
# Revision 1.1  2008/08/24 15:35:38  ntallen
# Diagnostics I want to port
#
# Revision 1.2  2008/07/23 13:07:42  nort
# Uncommitted changes and imports
#
# Revision 1.1  2001/07/24 18:50:33  nort
# Initial revision
#

TARGET=bckpln failtest scdebug sbswitch

LLIB=/usr/local/lib
CFLAGS=-Wall -g -I/usr/local/include
LDFLAGS=-Wl,-rpath -Wl,/usr/local/lib -L/usr/local/lib
LINK.priv=$(LINK.c) $(LDFLAGS) -o $@
LINK.norm=$(LINK.c) $(LDFLAGS) -o $@
LINK.ext=$(LINK.c) $(LDFLAGS) -o $@

all : $(TARGET)

bckpln : bckpln.o bckplnoui.o
	$(LINK.priv) bckpln.o bckplnoui.o -l ncurses -lnort
	usemsg bckpln bckplnoui.c
	/usr/local/sbin/promote bckpln
bckplnoui.o : bckplnoui.c bckpln.h
bckplnoui.c : bckpln.oui
	oui -o bckplnoui.c bckpln.oui

scdebug : scdebug.o $(LLIB)/diaglibs.lib
	$(LINK.priv) scdebug
	usemsg scdebug scdebug.c
	/usr/local/sbin/promote scdebug
scdebug.o : scdebug.c

failtest : failtest.o
	$(LINK.priv) failtest -l diaglib
	usemsg failtest failtest.c
	/usr/local/sbin/promote failtest
failtest.o : failtest.c

sbswitch.o : sbswitch.c
sbswitch : sbswitch.o
	$(LINK.priv) sbswitch
	usemsg sbswitch sbswitch.c
	/usr/local/sbin/promote sbswitch

.PHONY : clean install all
clean :
	rm -rf *.o *oui.c $(TARGET)
install : $(TARGET)
	@cp -vpn $(TARGET) /usr/local/bin