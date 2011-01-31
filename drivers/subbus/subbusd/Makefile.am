DIST_SUBDIRS = serusb syscon104
include_HEADERS = subbusd.h
bin_PROGRAMS = subbusd_serusb
AM_LDFLAGS = -L/usr/local/lib
AM_CPPFLAGS = -I/usr/local/include
LDADD = -lnort
subbusd_serusb_SOURCES = expint.c subbusd.c subbusd_int.h \
  serusb/serusb.c serusb/interrupt.c serusb/serusb.h \
  subbusd.h
subbusd_serusb_CPPFLAGS = -I serusb

subbusd_sc104_SOURCES = expint.c subbusd.c subbusd_int.h \
  sc104/sc104.c sc104/interrrupt.c sc104/hint.c
subbusd_sc104_CPPPFLAGS = -I sc104