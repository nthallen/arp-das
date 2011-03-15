LDFLAGS=-L /usr/local/lib -Wl,-rpath -Wl,/usr/local/lib
#LDFLAGS=-L /usr/local/lib
LIBS=-lsubbus -lnort
CPPFLAGS=-I /usr/local/include
CFLAGS=-Wall -g
CXXFLAGS=-Wall -g

sbtest : sbtest.o
	cc $(LDFLAGS) -o sbtest sbtest.o $(LIBS)
sbbench : sbbench.o
	cc $(LDFLAGS) -o sbbench sbbench.o $(LIBS)
install : ai_cfg.h
	@cp -vn ai_cfg.h /usr/local/include/
bench_ptrh : bench_ptrh.o ptrh.o
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o bench_ptrh bench_ptrh.o ptrh.o $(LIBS)
