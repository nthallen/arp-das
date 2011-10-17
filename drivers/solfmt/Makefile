# Makefile for solfmt
# $Log$
# Revision 1.6  2011/03/27 19:12:00  ntallen
# -rpath
#
# Revision 1.5  2011/02/24 01:32:16  ntallen
# Cleanup
#
# Revision 1.4  2011/02/24 00:57:59  ntallen
# Clean compile
#
# Revision 1.3  2011/02/22 18:40:36  ntallen
# Solfmt compiled
#
# Revision 1.2  2011/02/21 18:37:51  ntallen
# Tweak
#
# Revision 1.1  2011/02/21 18:26:05  ntallen
# QNX4 version
#
# Revision 1.1  1993/09/28  17:08:24  nort
# Initial revision
#
#
CFLAGS=-g -Wall
CPPFLAGS=-I /usr/local/include
LDFLAGS=-L /usr/local/lib -Wl,-rpath -Wl,/usr/local/lib
SRC=compile.c get_tokens.c optimize.c output.c read_cmd.c read_d2a.c
SRC+=read_mod.c read_sol.c read_val.c routines.c
SRC+=solfmt.c
SRC+=codes.h dtoa.h modes.h proxies.h routines.h
SRC+=solenoid.h solfmt.h tokens.h version.h
SRC+=read_pxy.c
TOOL=Makefile

SOURCE=$(SRC) $(TOOL)
OBJECT=$(OBJS) $(TEMP)
TARGET=solfmt
.PHONY : all clean install

all : $(TARGET)
OBJS=solfmt.o compile.o get_tokens.o optimize.o output.o
OBJS+=read_cmd.o read_d2a.o read_mod.o read_sol.o read_val.o
OBJS+=read_pxy.o routines.o
TEMP=*.sol *.sft

solfmt : $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o solfmt $(OBJS) -lnort
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

clean :
	rm -f *.o solfmt FullBuild.*
install : solfmt
	@cp -vn solfmt /usr/local/bin
