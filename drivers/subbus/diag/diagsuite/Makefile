.PHONY : all clean install

SRCS = $(wildcard *.c)
TARGETS = syscon card ana104 sc_cfg
SCOBJ = sccfg.o cfg.o
LDFLAGS = -L/usr/pkg/lib -L/usr/local/lib
CPPFLAGS = -I/usr/pkg/include -I/usr/local/lib

all : $(TARGETS)
syscon : $(SRCS)
	make -f Makefile.eil BASE=syscon
card : $(SRCS)
	make -f Makefile.eil BASE=card
ana104 : $(SRCS)
	make -f Makefile.eil BASE=ana104
sc_cfg : $(SCOBJ)
	$(LINK.c) -o sc_cfg $(SCOBJ) -lncurses
sccfg.o : sccfg/sccfg.c
	$(COMPILE.c) -I./ -o sccfg.o sccfg/sccfg.c
clean :
	rm -f *.o $(TARGETS)
install : $(TARGETS)
	cp -v $(TARGETS) $(TGTNODE)/usr/local/bin
	mkdir -p $(TGTNODE)/usr/local/share/huarp
	cp -v scdiag.cfg $(TGTNODE)/usr/local/share/huarp/
