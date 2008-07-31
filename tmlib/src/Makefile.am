lib_LTLIBRARIES = libtm.la
libtm_la_SOURCES = cic.c cictrans.c cis.c cis.h \
   tm_dev_name.c tm_open_name.c company.h cis_stubs.c timetext.c \
   tmctime.h colsend.c collect.h

include_HEADERS = tm.h tmctime.h collect.h

# Need to set CPPFLAGS when running configure (so it can find
#  cmdalgo.h) Then this is unnecessary
# AM_CPPFLAGS = -I /usr/local/include

ouidir = $(datadir)/oui
oui_HEADERS = cic.oui cis.oui msghdr.oui

tmcdir = $(datadir)/huarp
tmc_HEADERS = flttime.tmc root.cmd
