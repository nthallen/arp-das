LDFLAGS=-Wl,-rpath -Wl,/usr/local/lib -L/usr/local/lib
CPPFLAGS=-I/usr/local/include
CXXFLAGS=-Wall -g
CFLAGS=-Wall -g
LIBS=-ltmpp -ltm -lnort

OBJ = tmdf.o tmdfoui.o
.PHONY : clean all

all : tmdf

tmdf : $(OBJ)
	$(LINK.cc) -o tmdf $(OBJ) $(LIBS)
	usemsg tmdf tmdfoui.c
tmdfoui.c : tmdf.oui
	oui -o tmdfoui.c tmdf.oui

clean :
	@rm -f *.o tmdfoui.c tmdf
