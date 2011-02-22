# include definitions
include $(SRCDIR)/src.mk
SUBDIRS = 
HOMEDIR = $(DASDIR)/mods/sol/soldrv
BASE = soldrv

# file list macros
TXTS += $(USEDIR)/msg.txt $(USEDIR)/break.txt $(USEDIR)/cc.txt
# LIBS = eillib das
SRCS += read_sft.c
OBJS = soldrv.o read_sft.o
HDRS += sol.h
DOCS = toeil.txt
CFLAGS += -T0 -g

# include targets
include $(DASDIR)/tgt.mk

all: perms

# dependencies
soldrv.o : soldrv.c $(HDRS) ../h/codes.h ../h/version.h
read_sft.o : read_sft.c ../h/codes.h ../h/version.h sol.h
