CPPFLAGS = -I /usr/local/include
LDFLAGS = -Wl,-rpath -Wl,/usr/local/lib -L /usr/local/lib
CFLAGS = -Wall -g
LIBS=-lsubbus -lnort

.PHONY : all clean

all : pp_util

pp_util : pp_util.o
	$(LINK.c) -o pp_util pp_util.o $(LIBS)

clean :
	rm -f pp_util pp_util.o
