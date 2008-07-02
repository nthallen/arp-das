MEMO_OBJ=memo.o
CFLAGS=-I/usr/local/include
LDFLAGS=-L/usr/local/lib "-Wl,-rpath /usr/local/lib"
LIBS=-ltm -lnort
memo : $(MEMO_OBJ)
	$(CC) $(LDFLAGS) -o memo $(MEMO_OBJ) $(LIBS)
memo.o : memo.c memo.h

.PHONY : install clean all
all : memo
install : memo
	/usr/local/sbin/promote memo
	cp memo /usr/local/bin
clean :
	rm -f $(MEMO_OBJ) memo
