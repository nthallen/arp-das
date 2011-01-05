lib_LTLIBRARIES = libdisp.la libdispnc.la
libdisp_la_SOURCES = DCph.cc
libdisp_la_LDFLAGS = -version-info 0:0:0 -ltmph -lph
libdispnc_la_SOURCES = DCnc.cc
libdispnc_la_CPPFLAGS = -I/usr/pkg/include
libdispnc_la_LDFLAGS = -version-info 0:0:0 -ltmnc -L/usr/pkg/lib -lncurses
include_HEADERS = DCph.h DCnc.h

AM_LDFLAGS = -ltmpp -lnort

# This hack worked for 6.3.2, but not for 6.4.0
# CXX=cc
# And this one is removed for 6.5.0
# AM_CXXFLAGS=-lang-c++

