LDFLAGS=-Wl,-rpath -Wl,/usr/local/lib -L/usr/local/lib
CPPFLAGS=-I/usr/local/include
CXXFLAGS=-Wall -g
CFLAGS=-Wall -g
LIBS=-ltmpp -ltm -lnort
BINDIR=/usr/local/bin
HUARPDIR=/usr/local/share/huarp
INCDIR=/usr/local/include

OBJ = tmdf.o tmdfoui.o
.PHONY : clean all install

all : tmdf

tmdf : $(OBJ)
	$(LINK.cc) -o tmdf $(OBJ) $(LIBS)
	usemsg tmdf tmdfoui.c
tmdfoui.c : tmdf.oui
	oui -o tmdfoui.c tmdf.oui

clean :
	@rm -f *.o tmdfoui.c tmdf FullBuild*

install : tmdf tmdf.tmc tmdf_col.tmc
	@cp -vn tmdf $(BINDIR)
	@cp -vn tmdf.h $(INCDIR)
	@cp -vn tmdf.tmc tmdf_col.tmc $(HUARPDIR)
