SRC=depends.tmc caltest.tmc array.tmc stdint.tmc
TOOL=Makefile debug setup.dbg
DOC=todo

OBJ=depends.cc caltest.cc array.cc stdint.cc
TEMPORARY=*.err *.pcm

# Maintenance definitions
HOMEDIR=/usr/local/src/das/tmc/regress
MNC=tmcr
SOURCE=$(SRC) $(TOOL) $(DOC)
OBJECT=$(OBJ) $(INTERMED) $(TEMPORARY)

SUFFIXES= .o .cc .tmc
.SUFFIXES : $(SUFFIXES)
CXXFLAGS=-Wall -g -I /usr/local/include
CXXLINK=$(CXX)
LINK.args=$(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@
LINK.normCXX=$(CXXLINK) $(LINK.args)
LDFLAGS=-L/usr/local/lib
LIBS=-ltm -ltmpp -lnort
TMCFLAGS=-s
TMC=$(DBGR) ../tmc

.tmc.cc :
	$(TMC) $(TMCFLAGS) -o $@ -p -V $*.pcm -c $<

#-----------------------------------------------------------------
# Targets
#-----------------------------------------------------------------
.PHONY : regress clean
regress : $(OBJ)
	@echo All tests passed successfully
depends.cc : depends.tmc
caltest.cc : caltest.tmc
array.cc : array.tmc
array : array.o
	$(LINK.normCXX) array.o
clean :
	rm -f $(OBJ) *.o
