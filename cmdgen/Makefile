# Makefile for cmdgen
# $Id$
SRC=yyparse.y yylex.l cmdgen.h dstructs.c cmdgen.c states.c vunion.c
SRC+=trie.c prompts.c rules.c
SRC+=cmdgen.skel
TOOL=Makefile trial.cmd trial2.cmd
DOC=todo wrtest.c rdtest.c
OBJ=$(OBJMDL)/yyparse.o
OBJ+=$(OBJMDL)/yylex.o
OBJ+=$(OBJMDL)/dstructs.o
OBJ+=$(OBJMDL)/cmdgen.o
OBJ+=$(OBJMDL)/states.o
OBJ+=$(OBJMDL)/vunion.o
OBJ+=$(OBJMDL)/trie.o
OBJ+=$(OBJMDL)/prompts.o
OBJ+=$(OBJMDL)/rules.o
INTERMED=y.tab.h yyparse.c yylex.c trial.c trial.o trial
TEMPORARY=*.err rdtest.o wrtest.o rdtest wrtest setup.dbg

SOURCE=$(SRC) $(TOOL) $(DOC)
OBJECT=$(OBJ) $(INTERMED) $(TEMPORARY)
TARGET=cmdgen
HOMEDIR=/usr/local/src/das/cmdgen
MNC=cmdg

MODEL=c
OBJMDL=OBJ.$(MODEL)
MODELARGS=-m$(MODEL) -2
YFLAGS=-d -t -l
LFLAGS=-i
CFLAGS=$(MODELARGS) -fo$@ -w4 -g
LDFLAGS=$(BETALIB)
LSRC=/usr/local/lib/src

.INIT :
	@if [ $(MODEL) = 3r -a "$(MODELARGS)" != "-ms -3" ]; then\
	  echo Must invoke Model 3r via Makelib >&2;\
	  exit 1;\
	fi; :
	@if [ ! -d $(OBJMDL) ]; then mkdir $(OBJMDL); fi; :\

all : cmdgen $(LSRC)/cmdgen.skel
cmdgen : $(OBJ)
	$(LINK.c) -o cmdgen $(OBJ) $(LDFLAGS)
	usemsg cmdgen cmdgen.c
	ln -f cmdgen /usr/local/bin/cmdgen
$(OBJMDL)/yyparse.o : yyparse.c cmdgen.h
	$(COMPILE.c) -D lint yyparse.c
yyparse.c : yyparse.y
$(OBJMDL)/yylex.o : yylex.c cmdgen.h
yylex.c : yylex.l
$(OBJMDL)/dstructs.o : dstructs.c cmdgen.h
$(OBJMDL)/cmdgen.o : cmdgen.c cmdgen.h
$(OBJMDL)/states.o : states.c cmdgen.h
$(OBJMDL)/vunion.o : vunion.c cmdgen.h
$(OBJMDL)/trie.o : trie.c cmdgen.h
$(OBJMDL)/prompts.o : prompts.c cmdgen.h
$(OBJMDL)/rules.o : rules.c cmdgen.h
$(LSRC)/cmdgen.skel : cmdgen.skel
	ln -f cmdgen.skel $(LSRC)/cmdgen.skel

trial : $(OBJMDL)/trial.o
	$(CC) -g -ms -o trial $(OBJMDL)/trial.o $(LDFLAGS)
$(OBJMDL)/trial.o : trial.c
	$(CC) -c -g -ms trial.c
trial.c : trial.cmd cmdgen
	cmdgen <trial.cmd >trial.c
