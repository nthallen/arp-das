OBJ = crc16xmodem.o me_cmd.o me_query.o me_ser.o main.o
OBJ += meerstetteroui.o
CXXFLAGS=-I/usr/local/include
LDFLAGS = -Wl,-rpath -Wl,/usr/local/lib -L/usr/local/lib
LIBS = -ltmpp -ltm -lnort

meerstetter : $(OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o meerstetter $(OBJ) $(LIBS)
meerstetteroui.o : meerstetteroui.cc
meerstetteroui.cc : meerstetter.oui
	oui -o meerstetteroui.cc meerstetter.oui
