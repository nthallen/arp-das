include /usr/local/share/huarp/appgen.mk
CPPFLAGS=-I/usr/local/include
CXXFLAGS=-Wall -g
CFLAGS=-Wall -g
LDFLAGS=-lang-c++ -L/usr/local/lib -Wl,-rpath -Wl,/usr/local/lib

rdr : rdr.o rdroui.o quit_pulse.o
	$(LINK.priv) rdr.o rdroui.o quit_pulse.o -ltmpp -ltm -lnort
	usemsg rdr rdroui.c
	/usr/local/sbin/promote rdr
rdroui.c : rdr.oui
	$(OUI) -o rdroui.c rdr.oui

.PHONY : all clean install
all : rdr
install : rdr
	@cp -vpn rdr /usr/local/bin
clean :
	rm -f *.o rdroui.c rdr
