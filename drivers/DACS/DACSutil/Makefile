LDFLAGS=-L /usr/local/lib -Wl,-rpath -Wl,/usr/local/lib
#LDFLAGS=-L /usr/local/lib
LIBS=-lsubbus -lnort
CPPFLAGS=-I /usr/local/include
CFLAGS=-Wall -g
CXXFLAGS=-Wall -g

DACStest : sbtest.o
	cc $(LDFLAGS) -o DACStest sbtest.o $(LIBS)
sbbench : sbbench.o
	cc $(LDFLAGS) -o sbbench sbbench.o $(LIBS)
sbtest : sbtest.o
	cc $(LDFLAGS) -o sbtest sbtest.o $(LIBS)
install : ai_cfg.h ptrh.h DACStest
	@cp -vn ai_cfg.h ptrh.h /usr/local/include/
	@cp -vn DACStest /usr/local/bin
	@cp -vn DACS_ID.tmc /usr/local/share/huarp
bench_ptrh : bench_ptrh.o ptrh.o
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o bench_ptrh bench_ptrh.o ptrh.o $(LIBS)
