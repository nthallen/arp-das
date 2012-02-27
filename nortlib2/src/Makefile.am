lib_LTLIBRARIES = libnort.la
libnort_la_LDFLAGS = -version-info 1:0:1
libnort_la_SOURCES = nl_error.c nl_verr.c nldbg.c \
                     compiler.c ll_of_str.c newmem.c nlresp.c \
                     strdup.c opnifile.c optstr.c yyin.c md5c.c \
                     mlf.c skeleton.c rational.c dtoe.c ascii_esc.c
include_HEADERS = nortlib.h compiler.h ll_of_str.h md5.h mlf.h \
		  nl_assert.h rational.h dtoe.h
ouidir = $(datadir)/oui
oui_HEADERS = compiler.oui
bin_SCRIPTS = mlf_find
EXTRA_DIST = mlf_find
