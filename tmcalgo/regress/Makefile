HOMEDIR=/usr/local/src/das/tmcalgo/regress
MNC=tmar
TOOL=Makefile
SRC=regress.tma oldbug.tma
SOURCE=$(SRC) $(TOOL)
TMCOBJ=$(SRC:.tma=.tmc)
DOTOBJ=$(SRC:.tma=.dot)
OBJECT=$(TMCOBJ) $(DOTOBJ)
TMAREV=../tmcalgo -D $*.dot
.SUFFIXES: .tma .tmc
.tma.tmc:
	$(TMAREV) -o $@ $<

all : $(TMCOBJ)
clean :
	rm -f $(OBJECT)

$(TMCOBJ) : ../tmcalgo
