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
CFLAGS=-m$(MODEL) -w4 -g -I /usr/local/include
LDFLAGS=-b -l dbr -l das -l eillib -L /usr/local/lib
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
