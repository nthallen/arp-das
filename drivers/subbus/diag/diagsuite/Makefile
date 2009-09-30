.PHONY : all clean install

SRCS = $(wildcard *.c)
TARGETS = syscon card ana104

all : $(TARGETS)
syscon : $(SRCS)
	make -f Makefile.eil BASE=syscon
card : $(SRCS)
	make -f Makefile.eil BASE=card
ana104 : $(SRCS)
	make -f Makefile.eil BASE=ana104
clean :
	rm -f *.o $(TARGETS)
install : $(TARGETS)
	cp -v $(TARGETS) /usr/local/bin
