# Source Files
SRC=yylex.l grammar.y tmcalgo.c states.c commands.c
SRC+=yytype.h
TOOL=Makefile setup.dbg
DOC=todo algo grammar sample.tma oh_op.tma

# Derived Files
TA_OBJS=tmcalgo.o grammar.o yylex.o states.o commands.o
OBJ=$(TA_OBJS)
INTERMED=y.tab.h grammar.c yylex.c
TEMPORARY=*.err tmcalgo.map

# Maintenance definitions
SOURCE=$(SRC) $(TOOL) $(DOC)
OBJECT=$(OBJ) $(INTERMED) $(TEMPORARY)
TARGET=tmcalgo
HOMEDIR=/usr/local/src/das/tmcalgo
MNC=tmca
# SUBDIRS none

# Compilation options
YFLAGS=-d -l
LFLAGS=-i -l
CFLAGS=-g -ml -w4 -I /usr/local/include
LDFLAGS=-b -M -l nortlib -l memlib -L /usr/local/lib
LINK.it=$(LINK.c) -o $@ $(LDFLAGS)

tmcalgo : $(TA_OBJS)
	$(LINK.it) $(TA_OBJS)
tmcalgo.o : tmcalgo.c yytype.h
