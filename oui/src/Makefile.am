bin_PROGRAMS = oui
oui : ouiraw
	cp ouiraw oui
	usemsg oui ouiinit.c

EXTRA_PROGRAMS = ouiraw
CLEANFILES = ouiraw yylex.c ouigrmr.c ouigrmr.h
ouiraw_SOURCES = llop.c opnifile.c ouigrmr.y ouiinit.c ouimain.c output.c \
              sortpkg.c yylex.l
include_HEADERS = oui.h

CPPFLAGS = -I /usr/local/include
YFLAGS = -d
LFLAGS = -i -L
LDFLAGS = -lnort
