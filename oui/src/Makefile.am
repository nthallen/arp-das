bin_PROGRAMS = oui
include_HEADERS = oui.h
oui_SOURCES = llop.c opnifile.c ouigrmr.y ouiinit.c ouimain.c output.c \
              sortpkg.c yylex.l ouidefs.h
AM_CFLAGS = -D PKGDATADIR='"$(pkgdatadir)"'

CLEANFILES = ouiraw yylex.c ouigrmr.c ouigrmr.h

EXTRA_DIST = ouigrmr.h

AM_YFLAGS = -d
AM_LFLAGS = -i -L

# ouiinit.c is generated via oui
# oui -o ouiinit.c ouiinit.oui
 
if USE_USEMSG
all-local : oui
	usemsg oui $(srcdir)/ouiinit.c
endif
