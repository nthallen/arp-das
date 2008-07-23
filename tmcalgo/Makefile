# Source Files
SRC=yylex.l grammar.y tmcalgo.c states.c commands.c
SRC+=yytype.h tmcalgo.oui dot.c dot.h
TOOL=Makefile setup.dbg
DOC=todo algo grammar sample.tma oh_op.tma todo.R2

# Derived Files
TA_OBJS=tmcalgo.o grammar.o yylex.o states.o commands.o
TA_OBJS+=tmcalgooui.o dot.o
OBJ=$(TA_OBJS)
INTERMED=y.tab.h y.tab.c grammar.c yylex.c tmcalgooui.c
TEMPORARY=*.err tmcalgo.map sample.tmc

# Maintenance definitions
SOURCE=$(SRC) $(TOOL) $(DOC)
OBJECT=$(OBJ) $(INTERMED) $(TEMPORARY)
TARGET=tmcalgo
HOMEDIR=/usr/local/src/das/tmcalgo
MNC=tmca
SUBDIRS4=regress

# Compilation options
YFLAGS=-d -l -t
# Added the -t above for debugging
LFLAGS=-i -l
CFLAGS=-g -ms -3 -w4
LDFLAGS=$(BETALIB)
LINK.it=$(LINK.c) -o $@ $(LDFLAGS)

tmcalgo : $(TA_OBJS)
	$(LINK.it) $(TA_OBJS)
	usemsg tmcalgo tmcalgooui.c
	# ln -f tmcalgo /usr/local/bin/tmcalgo2
tmcalgo.o : tmcalgo.c yytype.h
tmcalgooui.o : tmcalgooui.c
tmcalgooui.c : tmcalgo.oui
	oui -o tmcalgooui.c tmcalgo.oui
y.output : grammar.y
	$(YACC.y) -v grammar.y
dot.o : dot.c dot.h
