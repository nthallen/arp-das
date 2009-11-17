SRC=specq.c specq.oui specq.h
LDFLAGS=-lunix -lsocket
CFLAGS=-w4 -g
specq : specq.o specqoui.o
	$(LINK.c) -o specq specq.o specqoui.o
	usemsg specq specqoui.c
specqoui.c : specq.oui
	oui -o specqoui.c specq.oui
