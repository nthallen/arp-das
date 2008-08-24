# to compile card:
#	make BASE=card
# to compile ana104
#	make BASE=ana104

# include $(SRCDIR)/src.mk
BASE = syscon
TXTS = syscon.txt
SRCS = diagutil.c suite.c syscon.c cfg.c stch_i.c stcpm.c stcpma.c
OBJS = $(SRCS:.c=.o)
LIBS = curses subbus lat curses_utils termlib
HDRS = attribut.h define.h diagmenu.h scdiag.h sic.h syscon.h
DOCS = diags.bak syscon.hlp scdiag.cfg
CFLAGS = -I/usr/local/include -Wall -g -D$(BASE)
LDFLAGS = -Wl,-rpath -Wl,/usr/local/lib -L/usr/local/lib

# include $(SRCDIR)/tgt.mk

# add perms to all

$(BASE) : $(OBJS)
	$(LINK.c) -o $(BASE) $(OBJS) -lm -lncurses -lsubbus -lnort
	/usr/local/sbin/promote $(BASE)
suite.o : suite.c syscon.h attribut.h scdiag.h 
diagutil.o: diagutil.c define.h

.PHONY : all clean install
clean :
	rm -f *.o
