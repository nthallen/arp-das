# Use appgen.mk to define AG_LDFLAGS, LINK.norm, OUI
include /usr/local/share/huarp/appgen.mk
OBJ=sspdrv.o udp.o tcp.o sspdrvoui.o
sspdrv : $(OBJ)
	$(LINK.norm) -o sspdrv $(OBJ) -lnort
sspdrvoui.c : sspdrv.oui
	$(OUI) sspdrv.oui
