lib_LTLIBRARIES = libtm.la
libtm_la_SOURCES = tm_open.c tm_stream.c tm_data.c tm_init.c \
   tm_quit.c tm_row.c tm_ts.c tm_info.c cic.c cictrans.c cis.c cis.h \
   tm_dev_name.c tm_open_name.c company.h cis_stubs.c timetext.c \
   tmctime.h
include_HEADERS = tm.h tmctime.h
AM_CPPFLAGS = -I /usr/local/include
ouidir = $(datadir)/oui
oui_HEADERS = cic.oui cis.oui msghdr.oui
tmcdir = $(datadir)/huarp
tmc_HEADERS = flttime.tmc
