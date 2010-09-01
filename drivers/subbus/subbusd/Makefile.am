DIST_SUBDIRS = serusb sc104
include_HEADERS = subbusd.h
bin_PROGRAMS = subbusd_serusb
AM_LDFLAGS = -L/usr/local/lib
AM_CPPFLAGS = -I/usr/local/include
LDADD = -lnort
subbusd_serusb_SOURCES = expint.c subbusd.c subbusd_int.h \
  serusb/serusb.c serusb/interrupt.c serusb/serusb.h \
  subbusd.h

