CPPFLAGS=-I /usr/local/include -Wall
CFLAGS=-g
LDFLAGS=-g -L /usr/local/lib -Wl,-rpath -Wl,/usr/local/lib
LDLIBS=-l nort -l tm
TMBFR_OBJ=TMbfr.o
TMbfr : $(TMBFR_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o TMbfr $(TMBFR_OBJ) $(LDLIBS)
	/usr/local/sbin/promote TMbfr
TMbfr.o : TMbfr.c TMbfr.h

.PHONY : clean
clean :
	rm TMbfr TMbfr.o
