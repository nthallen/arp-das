# Makefile for cmdgen
# $Id$
SRC=yyparse.y yylex.l cmdgen.h dstructs.c cmdgen.c states.c vunion.c
SRC+=typedefs.skel trie.c functions.skel prompts.c rules.c
SRC+=cmdgen.skel
TOOL=Makefile trial.cmd trial2.cmd
DOC=todo wrtest.c rdtest.c
OBJ=yyparse.o yylex.o dstructs.o cmdgen.o states.o vunion.o trie.o prompts.o
OBJ+=rules.o
INTERMED=y.tab.h yyparse.c yylex.c trial.c trial.o trial
TEMPORARY=*.err rdtest.o wrtest.o rdtest wrtest

SOURCE=$(SRC) $(TOOL) $(DOC)
OBJECT=$(OBJ) $(INTERMED) $(TEMPORARY)
TARGET=cmdgen
HOMEDIR=/usr/local/src/das/cmdgen
MNC=cmdg

MODEL=c
YFLAGS=-d -t -l
LFLAGS=-i
CFLAGS=-g -m$(MODEL) -w4 -l nortlib -L /usr/local/lib -I /usr/local/include

cmdgen : $(OBJ)
	$(LINK.c) -o cmdgen $(OBJ) $(LDFLAGS)
	usemsg cmdgen cmdgen.c
	ln -f cmdgen /usr/local/bin/cmdgen
yyparse.o : yyparse.c cmdgen.h
	$(COMPILE.c) -D lint yyparse.c
yyparse.c : yyparse.y
yylex.o : yylex.c cmdgen.h
yylex.c : yylex.l
dstructs.o : dstructs.c cmdgen.h
cmdgen.o : cmdgen.c cmdgen.h
states.o : states.c cmdgen.h
vunion.o : vunion.c cmdgen.h
trie.o : trie.c cmdgen.h
prompts.o : prompts.c cmdgen.h
rules.o : rules.c cmdgen.h

trial : trial.o
	$(CC) -g -ms -o trial trial.o $(LDFLAGS)
trial.o : trial.c
	$(CC) -c -g -ms trial.c
trial.c : trial.cmd cmdgen cmdgen.skel
	cmdgen <trial.cmd >trial.c
