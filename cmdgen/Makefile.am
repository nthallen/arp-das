BUILT_SOURCES = yyparse.h
bin_PROGRAMS = cmdgen
include_HEADERS = cmdalgo.h
pkgdata_DATA = cmdgen.skel
pkgdatadir=$(datarootdir)/huarp
cmdgen_SOURCES = cmdgen.c dstructs.c prompts.c rules.c \
  states.c trie.c vunion.c yyparse.y yylex.l cmdgen.h \
  interfaces.c

AM_YFLAGS=-d -t -l
AM_LFLAGS=-i

CVSCLEANFILES=Makefile.in aclocal.m4 autom4te.cache \
  config.h config.log config.status stamp-h1 \
  INSTALL depcomp install-sh missing ylwrap configure

.PHONY: cvsclean
cvsclean : maintainer-clean
	rm -f $(CVSCLEANFILES)
 
if USE_USEMSG
all-local : cmdgen
	usemsg cmdgen cmdgen.c
endif
