LDFLAGS=-L /usr/local/lib -Wl,-rpath -Wl,/usr/local/lib
#LDFLAGS=-L /usr/local/lib
LIBS=-lsubbus -lnort
CPPFLAGS=-I /usr/local/include
CFLAGS=-Wall -g

sbtest : sbtest.o
	cc $(LDFLAGS) -o sbtest sbtest.o $(LIBS)
sbbench : sbbench.o
	cc $(LDFLAGS) -o sbbench sbbench.o $(LIBS)
