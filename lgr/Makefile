include /usr/local/share/huarp/appgen.mk

lgr : lgr.o lgroui.o
	$(LINK.norm) -o lgr lgr.o lgroui.o -ltmpp -ltm -lnort -lang-c++
	usemsg lgr lgroui.c
lgr.o : lgr.cc
lgroui.c : lgr.oui
	oui -o lgroui.c lgr.oui
