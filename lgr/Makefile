include /usr/local/share/huarp/appgen.mk
CFLAGS=-Wall -g
CXXFLAGS=-Wall -g

lgr : lgr.o lgroui.o
	$(LINK.norm) -o lgr lgr.o lgroui.o -ltmpp -ltm -lnort -lang-c++
	usemsg lgr lgroui.c
lgr.o : lgr.cc
lgroui.c : lgr.oui
	oui -o lgroui.c lgr.oui
.PHONY : all install clean
all : lgr
install : lgr
	cp -vf lgr /usr/local/bin
clean :
	rm -f *.o lgr lgroui.c
