CPPFLAGS=-I /usr/local/include -Wall
CFLAGS=-g
LDFLAGS=-g -L /usr/local/lib -Wl,-rpath -Wl,/usr/local/lib
LDLIBS=-l nort -l tm
TMBFR_OBJ=TMbfr.o TMbfroui.o
TMbfr : $(TMBFR_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o TMbfr $(TMBFR_OBJ) $(LDLIBS)
	usemsg TMbfr TMbfroui.c
	/usr/local/sbin/promote TMbfr
TMbfr.o : TMbfr.c TMbfr.h
TMbfroui.c : TMbfr.oui
	oui -o TMbfroui.c TMbfr.oui

.PHONY : clean
clean :
	rm -f TMbfr TMbfr.o TMbfroui.o TMbfroui.c FullBuild.*
install :
	@cp -vp TMbfr /usr/local/bin
