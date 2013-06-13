LDFLAGS=-L /usr/local/lib -Wl,-rpath -Wl,/usr/local/lib
#LDFLAGS=-L /usr/local/lib
LIBS=-lsubbus -lnort
CPPFLAGS=-I /usr/local/include
CFLAGS=-Wall -g
CXXFLAGS=-Wall -g
REFFILES=DACS_ID.tmc DACS_AI.cmd ptrhm.cc ptrhm_col.cc
INCHDRS=ptrhm.h ptrh.h ai_cfg.h

DACStest : sbtest.o
	cc $(LDFLAGS) -o DACStest sbtest.o $(LIBS)
sbbench : sbbench.o
	cc $(LDFLAGS) -o sbbench sbbench.o $(LIBS)
sbtest : sbtest.o
	cc $(LDFLAGS) -o sbtest sbtest.o $(LIBS)
install : ai_cfg.h ptrh.h DACStest
	@cp -vn ptrhm.h /usr/local/include/
	@cp -vn DACStest /usr/local/bin
	@cp -vn $(REFFILES) /usr/local/share/huarp
bench_ptrh : bench_ptrh.o ptrh.o
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o bench_ptrh bench_ptrh.o ptrh.o $(LIBS)

qclivtest : qclivtest.o
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o qclivtest qclivtest.o $(LIBS)

.PHONY : clean
clean :
	rm -f DACStest *.o FullBuild.*

