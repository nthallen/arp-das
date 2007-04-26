BUILT_SOURCES = yyparse.h
bin_PROGRAMS = cmdgen
include_HEADERS = cmdalgo.h
cmdgen_SOURCES = cmdgen.c dstructs.c prompts.c rules.c \
  states.c trie.c vunion.c yyparse.y yylex.l
AM_YFLAGS=-d -t -l
AM_LFLAGS=-i
# These are set conditionally in configure.in, since they are
# required for the check for headers and libraries
# AM_LDFLAGS=-L/usr/local/lib
# cmdgen_LDADD=-lnort
