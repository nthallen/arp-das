YFLAGS=-d -t -l
CFLAGS=-g -M -ml -2 -w4
# To compile 32-bit, change above flags to -ms -3, but FIRST
# edit dbr.h to make tminfo struct model-independent. Otherwise
# generated tm.dac is erroneous.

LFLAGS=-i
LDFLAGS=$(BETALIB)
# SRC, TOOL and DOC are types of files which should be saved
SRC=tmc.y yylex.l tmcmain.c tmc.h yytype.h tmcstr.h parsfunc.c
SRC+=pdecls.c genpcm.c place.c pfuncs.c calibr.h
SRC+=calibr.c postproc.c pointers.c
SRC+=states.c decls.c decls.h
SRC+=colmain.skel extmain.skel
TOOL=Makefile defnt nonterms lit.awk fixline.awk
TOOL+=tm_cons.h
DOC=todo.96 todo guidelines Document todo.new changes tmc.syntax

# OBJ, INERMED and TEMPORARY are types of files which can be safely
# deleted during a "make tidy" operation
OBJ=tmc.o yylex.o tmcmain.o parsfunc.o pdecls.o genpcm.o place.o
OBJ+=pfuncs.o calibr.o postproc.o pointers.o states.o decls.o
INTERMED=tmc.nonterm tmc.term tmc.lit tmc.grammar y.tab.h yylex.c tmc.c
TEMPORARY=*.err y.output tmc.map regresstest
TEMPORARY+=source.txt sourcel.html source.html

# Maintenance definitions
SOURCE=$(SRC) $(TOOL) $(DOC)
OBJECT=$(OBJ) $(INTERMED) $(TEMPORARY)
TARGET=tmc
HOMEDIR=/usr/local/src/das/tmc
MNC=tmc
SUBDIRS=regress tmcr
LSRC=/usr/local/lib/src
LBIN=/usr/local/bin
NRTL=/usr/local/include/nortlib.h
TMCREV=tmc

#-----------------------------------------------------------------
# Targets
#-----------------------------------------------------------------
tmc : $(OBJ)
	$(LINK.c) -o tmc $(OBJ) $(LDFLAGS)
	usemsg tmc tmcmain.c
distribution : $(LBIN)/$(TMCREV) $(LSRC)/colmain.skel $(LSRC)/extmain.skel
regresstest : tmc regress
	@cd regress; $(MAKE) $(MAKEFLAGS) && touch ../regresstest
stats :
	wc $(SRC)
tmc.c : tmc.y
tmc.o : tmc.c tmcstr.h tmc.h yytype.h calibr.h
	$(COMPILE.c) -D lint tmc.c
y.output : tmc.y
	$(YACC.y) -v tmc.y
yylex.o : yylex.l tmcstr.h calibr.h yytype.h tmc.h $(NRTL)
	$(LEX.l) yylex.l | awk -f fixline.awk > yylex.c
	$(COMPILE.c) yylex.c
	$(RM) yylex.c
tmcmain.o : tmcmain.c tmc.h $(NRTL)
parsfunc.o : parsfunc.c tmcstr.h tmc.h yytype.h $(NRTL)
pdecls.o : pdecls.c tmcstr.h tmc.h
genpcm.o : genpcm.c tmcstr.h tmc.h $(NRTL)
place.o : place.c tmcstr.h tmc.h $(NRTL)
pfuncs.o : pfuncs.c tmcstr.h tmc.h
calibr.o : calibr.c tmcstr.h tmc.h calibr.h $(NRTL)
states.o : states.c tmcstr.h tmc.h $(NRTL)
pointers.o : pointers.c tmc.h $(NRTL)
postproc.o : postproc.c tmc.h tmcstr.h $(NRTL)
$(LSRC)/colmain.skel : colmain.skel
	ln -f colmain.skel $(LSRC)/colmain.skel
$(LSRC)/extmain.skel : extmain.skel
	ln -f extmain.skel $(LSRC)/extmain.skel
$(LBIN)/$(TMCREV) : tmc
	@echo Update $(LBIN)/$(TMCREV): \\c; \
	su -c "ln -f tmc $(LBIN)/$(TMCREV)"
