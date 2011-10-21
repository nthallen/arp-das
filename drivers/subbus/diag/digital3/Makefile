LDFLAGS=-L/usr/local/lib -Wl,-rpath,/usr/local/lib -lsubbus
CPPFLAGS=-I/usr/local/include
CFLAGS=-Wall -g

.PHONY : install clean
digital3 : digital3.o kbhit.o
	$(LINK.c) $(CFLAGS) -o digital3 digital3.o kbhit.o $(LDFLAGS)
clean :
	rm -f digital3 *.o FullBuild.*
install : digital3
	cp digital3 /usr/local/bin/digital3
