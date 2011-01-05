lib_LTLIBRARIES = libtmph.la libtmnc.la
libtmph_la_LDFLAGS = -version-info 0:0:0 -lph
libtmph_la_SOURCES = tablelib.c nlphcmd.c
libtmnc_la_CPPFLAGS = -I /usr/pkg/include
libtmnc_la_LDFLAGS = -version-info 0:0:0 -L/usr/pkg/lib -lncurses
libtmnc_la_SOURCES = nctable.c
include_HEADERS = tmph.h tablelib.h nlphcmd.h nctable.h
AM_LDFLAGS = -ltm -lnort

ouidir = $(datadir)/oui
oui_HEADERS = nctable.oui
