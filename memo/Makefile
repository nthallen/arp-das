MEMO_OBJ=memo.o
CFLAGS=-I/usr/local/include
LDFLAGS=-L/usr/local/lib -lnort "-Wl,-rpath /usr/local/lib"
memo : $(MEMO_OBJ)
memo.o : memo.c memo.h

.PHONY : install clean
install : memo
	cp memo /usr/local/bin
clean :
	rm $(MEMO_OBJ) memo
