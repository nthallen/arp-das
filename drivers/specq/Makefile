SRC=specq.c specq.oui specq.h
LDFLAGS=-L/usr/local/lib -lsocket -lnort -ltm
CPPFLAGS=-I /usr/local/include
CFLAGS=-w4 -g
specq : specq.o specqoui.o
	$(LINK.c) -o specq specq.o specqoui.o
	usemsg specq specqoui.c
specqoui.c : specq.oui
	oui -o specqoui.c specq.oui
.PHONY: all clean install install-bin
all : specq
clean :
	rm -f *.o specqoui.c specq
install : install-bin
	@cp -vn specq.h $(TGTNODE)/usr/local/include
install-bin : specq
	@cp -vn specq $(TGTNODE)/usr/local/bin
