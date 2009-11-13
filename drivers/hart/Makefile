SOURCE=Makefile hart.c doit
OBJECT=hart.o hart.log
TARGET=hart hartd
CPPFLAGS=-I/usr/local/include
CFLAGS=-g -Wall
LDFLAGS=-L/usr/local/lib -lnort -Wl,-rpath -Wl,/usr/local/lib
HARTD_LDFLAGS=-ltm

# sudo make install TGTNODE=/net/gsenode
# sudo make install-bin TGTNODE=/net/fltnode

.PHONY : all clean install install-bin install-dev

all : $(TARGET)

hart : hart.o gpib232.o
	$(LINK.c) -o hart hart.o gpib232.o
hartd : hartd.o hartdoui.o gpib232.o
	$(LINK.c) $(HARTD_LDFLAGS) -o hartd hartd.o hartdoui.o gpib232.o
	usemsg hartd hartdoui.c
hartd.o : hartd.c hartd.h
hartdoui.o : hartdoui.c hartd.h
hartdoui.c : hartd.oui gpib232.oui
	oui -o hartdoui.c hartd.oui

install : install-bin install-dev
install-bin : hartd
	@cp -vn hartd $(TGTNODE)/usr/local/bin
install-dev : hartd.h
	@cp -vn hartd.h $(TGTNODE)/usr/local/include
clean :
	rm -f $(TARGET) *.o hartdoui.c
