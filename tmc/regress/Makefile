SRC=depends.tmc caltest.tmc
TOOL=Makefile debug setup.dbg
DOC=todo

OBJ=depends.c caltest.c
TEMPORARY=*.err *.pcm

# Maintenance definitions
HOMEDIR=/usr/local/src/das/tmc/regress
MNC=tmcr
SOURCE=$(SRC) $(TOOL) $(DOC)
OBJECT=$(OBJ) $(INTERMED) $(TEMPORARY)

SUFFIXES= .o .c .tmc
.SUFFIXES : $(SUFFIXES)
MODEL=s
CFLAGS=-Wall -g -I /usr/local/include
LDFLAGS=-L/usr/local/lib
LIBS=-ltm -lnort
TMCFLAGS=-s
TMC=$(DBGR) ../tmc

.tmc.c :
	$(TMC) $(TMCFLAGS) -o $@ -p -V $*.pcm -c $<

#-----------------------------------------------------------------
# Targets
#-----------------------------------------------------------------
regress : $(OBJ)
	@echo All tests passed successfully
depends.c : depends.tmc
caltest.c : caltest.tmc
