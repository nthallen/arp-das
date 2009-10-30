HOMEDIR=/usr/local/src/das/hart
MNC=hart
SOURCE=Makefile hart.c doit
OBJECT=hart.o hart.log
TARGET=hart
TGTNODE=//22
TGTDIR=$(TGTNODE)/home/hart

DISTRIB=hart doit
all : $(TARGET)
distribution : $(DISTRIB)
	distribute $(TGTDIR) $(DISTRIB)

hartd : hartd.o hartdoui.o
	$(LINK.c) -o hartd hartd.o hartdoui.o
	usemsg hartd hartdoui.c
hartd.o : hartd.c hartd.h
hartdoui.o : hartdoui.c hartd.h
hartdoui.c : hartd.oui
	oui -o hartdoui.c hartd.oui
