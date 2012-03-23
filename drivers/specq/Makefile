SRC=specq.c specq.oui specq.h
LDFLAGS=-L/usr/local/lib -lsocket -lnort -ltm -Wl,-rpath -Wl,/usr/local/lib
CPPFLAGS=-I /usr/local/include
CFLAGS=-Wall -g
specq : specq.o specqoui.o
	$(LINK.c) -o specq specq.o specqoui.o
	usemsg specq specqoui.c
specqoui.c : specq.oui
	oui -o specqoui.c specq.oui
.PHONY: all clean install install-bin
all : specq
clean :
	rm -f *.o specqoui.c specq FullBuild.*
install : install-bin
	@[ -d $(TGTNODE)/usr/local/include ] && \
	cp -vn specq.h $(TGTNODE)/usr/local/include
install-bin : specq
	@cp -vn specq $(TGTNODE)/usr/local/bin
