OBJ = crc16xmodem.o me_cmd.o me_query.o me_ser.o
CXXFLAGS=-I/usr/local/include
LDFLAGS = -Wl,-rpath -Wl,/usr/local/lib -L/usr/local/lib

meerstetter : $(OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o meerstetter $(OBJ) $(LIBS)

