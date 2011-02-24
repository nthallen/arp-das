# Makefile for solfmt
# $Log$
# Revision 1.3  2011/02/22 18:40:36  ntallen
# Solfmt compiled
#
# Revision 1.2  2011/02/21 18:37:51  ntallen
# Tweak
#
# Revision 1.1  2011/02/21 18:26:05  ntallen
# QNX4 version
#
# Revision 1.3  2006/02/16 18:13:20  nort
# Uncommitted changes
#
# Revision 1.2  1993/09/28  17:15:28  nort
# *** empty log message ***
#
# Revision 1.1  1993/09/28  17:08:24  nort
# Initial revision
#
#
CFLAGS=-g -Wall
CPPFLAGS=-I /usr/local/include
LDFLAGS=-L /usr/local/lib
SRC=compile.c get_tokens.c optimize.c output.c read_cmd.c read_d2a.c
SRC+=read_mod.c read_sft.c read_sol.c read_val.c routines.c soldrv.c
SRC+=solfmt.c
SRC+=codes.h dtoa.h modes.h proxies.h routines.h solcall.h
SRC+=soldrv.h solenoid.h solfmt.h tokens.h version.h
SRC+=soldrv.txt solfmt.txt read_pxy.c
TOOL=Makefile

SOURCE=$(SRC) $(TOOL)
OBJECT=$(OBJS1) $(OBJS2) $(TEMP)
TARGET=solfmt

all : $(TARGET)
OBJS1=solfmt.o compile.o get_tokens.o optimize.o output.o
OBJS1+=read_cmd.o read_d2a.o read_mod.o read_sol.o read_val.o
OBJS1+=read_pxy.o routines.o
TEMP=*.err *.sol *.sft *.map soldrv.use

solfmt : $(OBJS1)
	$(CC) $(CFLAGS) $(LDFLAGS) -o solfmt $(OBJS1) -lnort
	usemsg solfmt solfmt.c

compile.o : compile.c solenoid.h codes.h
get_tokens.o : get_tokens.c tokens.h solenoid.h routines.h dtoa.h proxies.h
optimize.o : optimize.c dtoa.h solenoid.h modes.h
output.o : output.c solenoid.h modes.h version.h
read_cmd.o : read_cmd.c tokens.h
read_d2a.o : read_d2a.c tokens.h dtoa.h
read_mod.o : read_mod.c tokens.h solenoid.h modes.h routines.h
read_sol.o : read_sol.c tokens.h solenoid.h
read_val.o : read_val.c solenoid.h
routines.o : routines.c tokens.h routines.h
solfmt.o : solfmt.c
read_pxy.o : read_pxy.c proxies.h

OBJS2 = soldrv.o read_sft.o
soldrv : $(OBJS2) soldrv.use
	$(CC) $(CFLAGS) -o soldrv -l das_utils -l subbus $(OBJS2)
	usemsg soldrv soldrv.use

read_sft.o : read_sft.c soldrv.h codes.h version.h
soldrv.o : soldrv.c soldrv.h codes.h solcall.h

TXTS = soldrv.txt ../../utils/das_utils.txt

soldrv.use: $(TXTS)
	cat $(TXTS) >soldrv.use
