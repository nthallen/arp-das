# $Log$
# Revision 1.5  2010/09/10 13:09:14  ntallen
# Minor edit and relink for libsubbus.so.1
#
# Revision 1.4  2009/09/30 15:19:30  ntallen
# Removed scdebug from target list
#
# Revision 1.3  2008/09/09 13:20:44  ntallen
# Port failtest to NTO
#
# Revision 1.2  2008/08/28 15:09:19  ntallen
# Compiled and running
#
# Revision 1.1  2008/08/24 15:35:38  ntallen
# Diagnostics I want to port
#
# Revision 1.2  2008/07/23 13:07:42  nort
# Uncommitted changes and imports
#
# Revision 1.1  2001/07/24 18:50:33  nort
# Initial revision
#

# TARGET=bckpln failtest scdebug sbswitch
TARGET=bckpln failtest sbswitch

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
	$(LINK.priv) failtest.o -lsubbus -lnort
	usemsg failtest failtest.c
failtest.o : failtest.c

sbswitch.o : sbswitch.c
sbswitch : sbswitch.o
	$(LINK.norm) sbswitch.o -lsubbus -lnort
	usemsg sbswitch sbswitch.c

.PHONY : clean install all
clean :
	rm -rf *.o *oui.c $(TARGET)
install : $(TARGET)
	@cp -vpn $(TARGET) /usr/local/bin
