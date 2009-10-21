SUBDIRS=config IdxC
TOOL=Makefile todo
SRC=idx64.c idx64int.h
SRC+=idx64.oui
OBJ=idx64.o idx64oui.o hint.o expint.o
INTERM=idx64oui.c
TEMP=*.err

SOURCE=$(SRC) $(TOOL)
OBJECT=$(OBJ) $(INTERM) $(TEMP)
TARGET=idx64

BIN=idx64
DATA=idx64.cmd
BINDIR=$(TGTNODE)/usr/local/bin
DATADIR=$(TGTNODE)/usr/local/share/huarp

CPPFLAGS=-I /usr/local/include
CFLAGS=-Wall -g
LDFLAGS=-L/usr/local/lib -Wl,-rpath -Wl,/usr/local/lib
LIBS=-lsubbus -lnort -ltm


idx64 : $(OBJ)
	rm -f idx64
	$(CC) $(CFLAGS) $(LDFLAGS) -o idx64 $(OBJ) $(LIBS)
	usemsg idx64 idx64oui.c
	/usr/local/sbin/promote idx64
idx64.o : idx64.c idx64int.h idx64.h
idx64oui.o : idx64oui.c idx64int.h
idx64oui.c : idx64.oui
	oui -o idx64oui.c idx64.oui
expint.o : expint.c
hint.o : hint.c

.PHONY : all clean install install-bin install-data install-config
all : idx64
clean :
	rm -f *.o idx64oui.c idx64
install : install-bin install-data install-config
install-bin : $(BIN)
	@cp -v $(BIN) $(BINDIR)
install-data : $(DATA)
	@cp -v $(DATA) $(DATADIR)
install-config :
	( cd config; make install TGTNODE=$(TGTNODE) )
