MNC=idx64
HOMEDIR=/usr/local/src/das/idx64
SUBDIRS=config IdxC
TOOL=Makefile todo indexer.c
SRC=idx64.c idx64int.h
SRC+=idx64.oui
OBJ=idx64.o idx64oui.o
INTERM=idx64oui.c
TEMP=*.err

SOURCE=$(SRC) $(TOOL)
OBJECT=$(OBJ) $(INTERM) $(TEMP)
TARGET=idx64

CFLAGS=-ms -2 -w4 -g
LDFLAGS=-b -l eillib
LINC=/usr/local/include

idx64 : $(OBJ)
	$(LINK.c) -T1 -o idx64 $(OBJ) $(LDFLAGS)
	usemsg idx64 idx64oui.c
	promote idx64
idx64.o : idx64.c idx64int.h $(LINC)/idx64.h
idx64oui.o : idx64oui.c idx64int.h
idx64oui.c : idx64.oui
	oui -o idx64oui.c idx64.oui
