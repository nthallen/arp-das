TOOL=Makefile todo
SRC=omsdrv.c omsirq.c omsqueue.c omsdrv.oui
SRC+=omsdrv.h omsint.h
SRC+=testoms.c testdrv.c
DRVOBJ=omsdrv.o omsirq.o omsqueue.o omsdrvoui.o
OBJ=testoms.o testdrv.o
OBJ+=omsdrvoui.c
OBJ+=$(DRVOBJ)
TEMP=*.err

SOURCE=$(SRC) $(TOOL)
OBJECT=$(OBJ) $(TEMP)
TARGET=omsdrv testoms testdrv

CFLAGS=-Wall -g
LDFLAGS=-L /usr/local/lib -l tm -lnort

include /usr/local/share/huarp/appgen.mk

.PHONY : install

omsdrv : $(DRVOBJ)
	rm -f omsdrv
	$(LINK.priv) $(DRVOBJ)
	usemsg omsdrv omsdrvoui.c
	/usr/local/sbin/promote omsdrv
omsdrv.o : omsdrv.c omsint.h omsdrv.h
omsirq.o : omsirq.c omsint.h
#	$(COMPILE.c) -zu -Wc,-s -o omsirq.o omsirq.c
omsqueue.o : omsqueue.c omsint.h
omsdrvoui.c : omsdrv.oui
	$(OUI) omsdrv.oui

install :
	cp omsdrv /usr/local/bin
	cp omsdrv.h /usr/local/include

testoms : testoms.o
	rm -f testoms
	$(LINK.priv) testoms.o
	/usr/local/sbin/promote testoms

testdrv : testdrv.o
	$(LINK.norm) testdrv.o
testdrv.o : testdrv.c
