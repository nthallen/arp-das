SRC=yyparse.y yylex.l cmdgen.h dstructs.c cmdgen.c states.c vunion.c
SRC+=typedefs.skel trie.c functions.skel prompts.c rules.c
TOOL=Makefile trial.cmd trial2.cmd
DOC=todo
OBJ=yyparse.o yylex.o dstructs.o cmdgen.o states.o vunion.o trie.o prompts.o
OBJ+=rules.o
INTERMED=y.tab.h yyparse.c yylex.c cmdgen trial.c trial.o trial
TEMPORARY=*.err
SAVE=$(SRC) $(TOOL) $(DOC)
PURGE=$(OBJ) $(INTERMED) $(TEMPORARY)
MNC=cmdg

YFLAGS=-d -t -l
LFLAGS=-i
CFLAGS=-g -ms

trial : trial.o
	$(LINK.c) -o trial trial.o $(LDFLAGS)
trial.o : trial.c
trial.c : trial.cmd cmdgen typedefs.skel functions.skel
	cmdgen <trial.cmd >trial.c
cmdgen : $(OBJ)
	$(LINK.c) -o cmdgen $(OBJ) $(LDFLAGS)
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

#-----------------------------------------------------------------
# Maintainance targets:
#	make clean
#	make exchange
#	make backup
#	make archive
#	make backup archive
#	make update
#	make archlist
# This set of commands should be entirely generic given the
# definition of the following macros:
#   MNC the name of the backup directory. This should be
#       unique system-wide, since it will probably go into
#       the same directory with all the other backup directories
#   SUBDIRS list of subdirectories that should be grouped together
#       with this one for maintenance purposes.
#   INCLUDED list of files from /usr/local/include which are
#       used in this directory. This will be copied together
#       with other source files into an "included" subdirectory
#       when the backup target is invoked.
#	SAVE list of files which are to be saved during backup
#	PURGE list of files which are to be deleted during cleanup
#	SAVE and PURGE should list all files in the directory.
#		make unlisted (or exchange) will alert you to any
#		files not listed in one or the other category.
#-----------------------------------------------------------------
# BACKUPDIR is where duplicate copies of source files are kept
BACKUPROOT=$(TMPDIR)
BACKUPDIR=$(BACKUPROOT)/$(MNC)
ULRT=$(TMPDIR)/$(MNC)
RECURSE=if test -n "$(SUBDIRS)"; then\
	  for i in $(SUBDIRS); do\
	    if (cd $$i; $(MAKE) $(MAKEFLAGS) BACKUPROOT=$(BACKUPDIR) $@);\
		then :;\
		else exit 1;\
		fi;\
	  done;\
	else :;\
	fi;

# make clean reduces to source code only
clean :
	@$(RECURSE)
	rm $(PURGE)
	rm -r $(BACKUPDIR)

# make exchange exchanges newer versions of source with a backup directory.
# make backup copies the RCS files also. make backup archive should therefore
# backup the current source AND the RCS files.
exchange :
	@cp -civn $(SAVE) $(BACKUPDIR); :
	@cd $(BACKUPDIR); find . -level 1 -a -type f | xargs -i cp -civn {} $(PWD); :
	@$(RECURSE)
backup : exchange
	@if test -n "$(INCLUDED)"; then\
	  cd /usr/local/include; cp -cvn $(INCLUDED) $(BACKUPDIR)/included;\
	fi; :
	@if test -d RCS; then\
	  cp -cvn RCS/* $(BACKUPDIR)/RCS;\
	fi; :

# make archive will copy all the source stuff to a floppy archive.
archive : unlisted exchange
	cd $(BACKUPDIR); pax -wv . | freeze | vol -w /dev/fd0
	if test -d $(BACKUPDIR)/RCS; then rm -rf $(BACKUPDIR)/RCS; fi; :
	if test -d $(BACKUPDIR)/included; then rm -rf $(BACKUPDIR)/included; fi; :

# make update should read an archive off of floppy into the BACKUPDIR
# make update exchange should then exchange that info with the current
# info. It does not copy the RCS stuff if it was a "backup archive"
update :
	if test -d $(BACKUPDIR); then : ; else mkdir $(BACKUPDIR); fi; :
	cd $(BACKUPDIR); vol -r /dev/fd0 | melt | pax -rov
archlist :
	vol -r /dev/fd0 | melt | pax -v

# make unlisted determines whether the macros for files to be saved
# or purged define all the files in the directory.
unlisted :
	@echo Checking for unlisted files in $(PWD)/Makefile:
	@find . -level 1 -a -type f -a ! -name \*.TMP | sort >$(ULRT)files
	@(for i in $(SAVE) $(PURGE); do echo ./$$i; done) | sort >$(ULRT)list
	@diff $(ULRT)files $(ULRT)list | grep "^<" > $(ULRT)unlisted; :
	@rm $(ULRT)files $(ULRT)list
	@if test -s $(ULRT)unlisted; then cat $(ULRT)unlisted; rm $(ULRT)unlisted; false; else rm $(ULRT)unlisted; fi
	@$(RECURSE)
