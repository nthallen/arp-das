# include definitions
CFLAGS=-g -Wall
LDFLAGS=-L /usr/local/lib -Wl,-rpath -Wl,/usr/local/lib
CPPFLAGS=-I /usr/local/include -I ../solfmt
OBJS=soldrv.o read_sft.o soldrvoui.o

.PHONY : all clean install

soldrv : $(OBJS)
	$(LINK.c) -o soldrv $(OBJS) -lsubbus -ltm -lnort
	usemsg soldrv soldrvoui.c
soldrv.o : soldrv.c sol.h ../solfmt/version.h
read_sft.o : read_sft.c sol.h ../solfmt/version.h
soldrvoui.o : soldrvoui.c
soldrvoui.c : soldrv.oui
	oui -o soldrvoui.c soldrv.oui

install : soldrv
	@cp -vn soldrv /usr/local/bin

clean :
	rm -f *.o soldrv
all : soldrv
