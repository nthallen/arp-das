lib_LTLIBRARIES = libtm.la
libtm_la_LDFLAGS = -version-info 0:0:0
libtm_la_SOURCES = cic.c cicf.c cictrans.c cis.c cis.h \
   tm_dev_name.c tm_open_name.c company.h cis_stubs.c timetext.c \
   tmctime.h colsend.c collect.h tm_info.c msg.c ci_cmdee.c civ.c \
   tma.c tma_file.c tmaR2.c tma_stubs.c tmdac.c

include_HEADERS = tm.h tmctime.h collect.h hsatod.h msg.h tma.h \
   company.h

# Need to set CPPFLAGS when running configure (so it can find
#  cmdalgo.h) Then this is unnecessary
# AM_CPPFLAGS = -I /usr/local/include

ouidir = $(datadir)/oui
oui_HEADERS = cic.oui cis.oui msghdr.oui msg.oui kbdclt.oui civ.oui \
  tma.oui collect.oui

tmcdir = $(datadir)/huarp
tmc_HEADERS = flttime.tmc root.cmd getcon.cmd
