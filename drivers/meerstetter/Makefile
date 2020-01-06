OBJ = crc16xmodem.o me_cmd.o me_query.o me_ser.o main.o
OBJ += meerstetteroui.o
CXXFLAGS=-I/usr/local/include -g -Wall
LDFLAGS = -Wl,-rpath -Wl,/usr/local/lib -L/usr/local/lib
LIBS = -ltmpp -ltm -lnort

.PHONY : all clean install

all : meerstetter

meerstetter : $(OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o meerstetter $(OBJ) $(LIBS)
	usemsg -c meerstetter meerstetteroui.cc
meerstetteroui.o : meerstetteroui.cc
meerstetteroui.cc : meerstetter.oui
	oui -o meerstetteroui.cc meerstetter.oui

clean :
	rm -f *.o meerstetteroui.cc meerstetter
install :
	@cp -vn meerstetter $(TGTNODE)/usr/local/bin
	@cp -vn meerstetter.h $(TGTNODE)/usr/local/include
	@cp -vn TM/Meerstetter.agm $(TGTNODE)/usr/local/share/huarp
	@cp -vn TM/Meerstetter_gen $(TGTNODE)/usr/local/share/huarp
