MEMO_OBJ=memo.o memooui.o
CFLAGS=-I/usr/local/include -Wall
LDFLAGS=-L/usr/local/lib "-Wl,-rpath /usr/local/lib"
LIBS=-ltm -lnort
memo : $(MEMO_OBJ)
	$(CC) $(LDFLAGS) -o memo $(MEMO_OBJ) $(LIBS)
	usemsg memo memooui.c
	/usr/local/sbin/promote memo
memo.o : memo.c memo.h
memooui.c : memo.oui
	oui -o memooui.c memo.oui

.PHONY : install clean all
all : memo
install : memo
	@cp -vp memo /usr/local/bin
clean :
	rm -f $(MEMO_OBJ) memo FullBuild.*
