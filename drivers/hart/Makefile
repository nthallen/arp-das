SOURCE=Makefile hart.c doit
OBJECT=hart.o hart.log
TARGET=hart
TGTDIR=$(TGTNODE)/home/hart
CPPFLAGS=-I/usr/local/include
CFLAGS=-g -Wall
LDFLAGS=-L/usr/local/lib -lnort -Wl,-rpath -Wl,/usr/local/lib

DISTRIB=hart doit
all : $(TARGET)
distribution : $(DISTRIB)
	distribute $(TGTDIR) $(DISTRIB)

hart : hart.o gpib232.o
	$(LINK.c) -o hart hart.o gpib232.o
hartd : hartd.o hartdoui.o gpib232.o
	$(LINK.c) -o hartd hartd.o hartdoui.o gpib232.o
	usemsg hartd hartdoui.c
hartd.o : hartd.c hartd.h
hartdoui.o : hartdoui.c hartd.h
hartdoui.c : hartd.oui
	oui -o hartdoui.c hartd.oui
