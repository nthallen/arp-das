lib_LTLIBRARIES = libtm.la
libtm_la_SOURCES = tm_open.c tm_stream.c tm_data.c tm_init.c \
   tm_quit.c tm_row.c tm_ts.c tm_info.c cic.c cictrans.c cis.c cis.h \
   tm_dev_name.c tm_open_name.c company.h cis_stubs.c timetext.c \
   tmctime.h colsend.c collect.h DC.cc DC.h DG.cc DG.h DG_Resmgr.cc \
   DG_Resmgr.h DG_cmd.cc DG_cmd.h DG_data.cc DG_data.h DG_tmr.cc \
   DG_tmr.h DCcol.cc DGcol.h

include_HEADERS = tm.h tmctime.h collect.h DC.h DG.h DG_Resmgr.h \
   DG_cmd.h DG_data.h DG_tmr.h DQ.h DGcol.h

AM_CPPFLAGS = -I /usr/local/include
ouidir = $(datadir)/oui
oui_HEADERS = cic.oui cis.oui msghdr.oui
tmcdir = $(datadir)/huarp
tmc_HEADERS = flttime.tmc root.cmd
