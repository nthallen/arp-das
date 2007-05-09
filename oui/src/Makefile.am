bin_PROGRAMS = oui
include_HEADERS = oui.h
oui : ouiraw
	cp ouiraw oui
	$(USEMSG) oui $(srcdir)/ouiinit.c
oui_SOURCES =
AM_CFLAGS = -D PKGDATADIR='"$(pkgdatadir)"'

EXTRA_PROGRAMS = ouiraw
CLEANFILES = ouiraw yylex.c ouigrmr.c ouigrmr.h
ouiraw_SOURCES = llop.c opnifile.c ouigrmr.y ouiinit.c ouimain.c output.c \
              sortpkg.c yylex.l ouidefs.h

EXTRA_DIST = ouigrmr.h

AM_YFLAGS = -d
AM_LFLAGS = -i -L

# ouiinit.c is generated via oui
# oui -o ouiinit.c ouiinit.oui
