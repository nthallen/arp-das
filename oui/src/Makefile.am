bin_PROGRAMS = oui
include_HEADERS = oui.h
oui : ouiraw
	cp ouiraw oui
	$(USEMSG) oui ouiinit.c
oui_SOURCES =

EXTRA_PROGRAMS = ouiraw
CLEANFILES = ouiraw yylex.c ouigrmr.c ouigrmr.h
ouiraw_SOURCES = llop.c opnifile.c ouigrmr.y ouiinit.c ouimain.c output.c \
              sortpkg.c yylex.l ouidefs.h

EXTRA_DIST = ouigrmr.h

YFLAGS = -d
LFLAGS = -i -L
