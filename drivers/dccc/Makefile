SRCS = dccc.c dc_read.c dcccoui.c dccc_if.c
OBJS = $(SRCS:.c=.o)
LDFLAGS = -L/usr/local/lib -Wl,-rpath -Wl,/usr/local/lib
LIBS = -ltm -lsubbus -lnort
CFLAGS=-Wall -g

include /usr/local/share/huarp/appgen.mk

all : dccc
dccc : $(OBJS)
	rm -f dccc
	$(LINK.norm) $(OBJS) $(LIBS)
	usemsg dccc dcccoui.c
	/usr/local/sbin/promote dccc
dc_read.o: dc_read.c disc_cmd.h
dccc.o: dccc.c disc_cmd.h
dcccoui.c : dccc.oui
	oui -o dcccoui.c dccc.oui

.PHONY : clean install all
clean :
	rm -f *.o dcccoui.c dccc
install : dccc
	@cp -vp dccc $(TGTNODE)/usr/local/bin
