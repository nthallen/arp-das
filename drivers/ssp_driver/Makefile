# Use appgen.mk to define AG_LDFLAGS, LINK.norm, OUI
CFLAGS=-Wall -g
include /usr/local/share/huarp/appgen.mk
OBJ=sspdrv.o udp.o tcp.o sspdrvoui.o
CLEANFILES=$(OBJ) sspdrvoui.c sspdrv FullBuild.*
sspdrv : $(OBJ)
	$(LINK.norm) -o sspdrv $(OBJ) -lm -ltm -lnort -lsocket
	usemsg sspdrv sspdrvoui.c
sspdrvoui.c : sspdrv.oui
	$(OUI) sspdrv.oui
udp.o : udp.c sspint.h ssp_ad.h sspdrv.h
tcp.o : tcp.c sspint.h ssp_ad.h sspdrv.h
sspdrv.o : sspdrv.c sspint.h ssp_ad.h sspdrv.h

.PHONY : clean install
clean :
	rm -f $(CLEANFILES)
install : sspdrv sspdrv.h
	@cp -nv sspdrv /usr/local/bin
	@cp -nv sspdrv.h /usr/local/include
