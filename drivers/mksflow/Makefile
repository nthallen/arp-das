OBJ = mks_cmd.o mks_query.o mks_ser.o main.o
OBJ += mksflowoui.o
CXXFLAGS=-I/usr/local/include -g -Wall
LDFLAGS = -Wl,-rpath -Wl,/usr/local/lib -L/usr/local/lib
LIBS = -ltmpp -ltm -lnort

.PHONY : all clean install

all : mksflow

mksflow : $(OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o mksflow $(OBJ) $(LIBS)
	usemsg -c mksflow mksflowoui.cc
mksflowoui.o : mksflowoui.cc
mksflowoui.cc : mksflow.oui
	oui -o mksflowoui.cc mksflow.oui

clean :
	rm -f *.o mksflowoui.cc mksflow
	rm -rf eng
	cd TM/test && make clean
	cd Diag && make clean
install : mksflow
	@cp -vn mksflow $(TGTNODE)/usr/local/bin
	@cp -vn mksflow.h $(TGTNODE)/usr/local/include
	@cp -vn TM/mksflow.agm $(TGTNODE)/usr/local/share/huarp
	@cp -vn TM/mksflow_gen $(TGTNODE)/usr/local/share/huarp
