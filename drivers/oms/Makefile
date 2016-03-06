TOOL=Makefile todo
SRC=omsdrv.c omsirq.c omsqueue.c omsdrv.oui
SRC+=omsdrv.h omsint.h
SRC+=testoms.c testdrv.c
SRC+=OMS.cmd
DRVOBJ=omsdrv.o omsirq.o omsqueue.o omsdrvoui.o
MODSRC=OMS.agm OMS.cmd OMS.tmc.in OMS_col.tmc OMS.genui.in
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

.PHONY : install clean

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

install : omsdrv omsdrv.h OMS.cmd
	@if [ `id -u` != 0 ]; then echo Must be root to install; exit 1; fi
	@cp -vn omsdrv /usr/local/bin
	@cp -vn omsdrv.h /usr/local/include
	@cp -vn $(MODSRC) /usr/local/share/huarp

clean :
	rm -f omsdrv *.o omsdrvoui.c FullBuild.*
