# to compile card:
#	make BASE=card
# to compile ana104
#	make BASE=ana104

include $(SRCDIR)/src.mk
#BASE = syscon
SUBDIRS =
HOMEDIR = $(SRCDIR)/diags/suite
TXTS = syscon.txt
SRCS = diagutil.c suite.c syscon.c
LIBS = curses subbus lat curses_utils termlib
HDRS = attribut.h define.h diagmenu.h scdiag.h sic.h syscon.h
DOCS = diags.bak syscon.hlp scdiag.cfg
CFLAGS += -D$(BASE) -T1
LDFLAGS += -T1

include $(SRCDIR)/tgt.mk

# add perms to all
all : perms

suite.o : suite.c syscon.h attribut.h scdiag.h 
diagutil.o: diagutil.c define.h
