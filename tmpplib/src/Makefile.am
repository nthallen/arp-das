lib_LTLIBRARIES = libtmpp.la
libtmpp_la_LDFLAGS = -version-info 2:0:1
libtmpp_la_SOURCES = DC.cc DC.h DG.cc DG.h DG_Resmgr.cc \
   DG_Resmgr.h DG_cmd.cc DG_cmd.h DG_data.cc DG_data.h DG_tmr.cc \
   DG_tmr.h DGcol.cc DGcol.h DQ.cc DQ.h tminitfunc.cc \
   cis.cc cis.h \
   Selector.h Selector.cc Selectee.cc Timeout.h Timeout.cc \
   SerSelector.h SerSelector.cc \
   cpu_usage.cc cpu_usage.h \
   csv_file.cc csv_file.h

include_HEADERS = DC.h DG.h DG_Resmgr.h DG_cmd.h DG_data.h \
  DG_tmr.h DQ.h DGcol.h Selector.h SerSelector.h Timeout.h \
  cpu_usage.h csv_file.h

ouidir = $(datadir)/oui
oui_HEADERS = dc.oui
huarpdir = $(datadir)/huarp
huarp_HEADERS = cpu_usage.tmc cpu_usage_col.tmc

# Need to set CPPFLAGS in configure, so this isn't necessary:
# AM_CPPFLAGS = -I /usr/local/include

# CXX = QCC
# Commenting out for QNX 6.5.0
#AM_CXXFLAGS = -lang-c++
