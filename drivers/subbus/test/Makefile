LDFLAGS=-Wl,-rpath -Wl,/usr/local/lib -L/usr/local/lib
CPPFLAGS=-I/usr/local/include
CXXFLAGS=-Wall -g
CFLAGS=-Wall -g
LIBS=-ltmpp -lsubbuspp -ltm -lnort
BINDIR=/usr/local/bin
HUARPDIR=/usr/local/share/huarp
INCDIR=/usr/local/include

OBJ = test_bmm.o test_sboui.o
HALOBJ = test_hal_bmm.o test_haloui.o
TCOBJ = test_can.o test_sboui.o
.PHONY : clean all install

all : test_bmm test_hal_bmm test_can

test_bmm : $(OBJ)
	$(LINK.cc) -o test_bmm $(OBJ) $(LIBS)
	usemsg test_bmm test_sboui.cc
test_sboui.cc : test_sb.oui
	oui -o test_sboui.cc test_sb.oui
test_hal_bmm : $(HALOBJ)
	$(LINK.cc) -o test_hal_bmm $(HALOBJ) $(LIBS)
	usemsg test_hal_bmm test_haloui.cc
test_haloui.cc : test_hal.oui test_sb.oui
	oui -o test_haloui.cc test_hal.oui
test_can : $(TCOBJ)
	$(LINK.cc) -o test_can $(TCOBJ) $(LIBS)
	usemsg test_can test_sboui.cc

clean :
	@rm -f *.o test_sboui.cc test_bmm test_haloui.cc test_hal_bmm FullBuild*
