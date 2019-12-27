include_HEADERS = subbusd.h
bin_PROGRAMS = subbusd_serusb subbusd_sc104 subbusd_sim subbusd_CAN
LDFLAGS = -L/usr/local/lib -Wl,-rpath -Wl,/usr/local/lib
CPPFLAGS = -I ../lib -I .
LDADD = -ltm -lnort
CFLAGS=-Wall -g

.PHONY : all install clean
all : $(bin_PROGRAMS)
clean :
	rm -f *.o $(bin_PROGRAMS) sc104oui.c serusboui.c sbsimoui.c
install :
	@cp -vn $(bin_PROGRAMS) /usr/local/bin
	@cp -vn $(include_HEADERS) /usr/local/include

subbusd_serusb_SOURCES = expint.c subbusd.c subbusd_int.h \
  serusb/serusb.c serusb/interrupt.c serusb/serusb.h \
  subbusd.h serusboui.c cache.c client_buf_default.c
CPPFLAGS += -I serusb
subbusd_serusb_OBJECTS = expint.o subbusd.o serusb_serusb.o \
  serusb_interrupt.o serusboui.o cache.o client_buf_default.o

subbusd_serusb : $(subbusd_serusb_OBJECTS)
	$(LINK.c) -o subbusd_serusb $(LDADD) $(subbusd_serusb_OBJECTS)
	usemsg subbusd_serusb serusboui.c
	/usr/local/sbin/promote subbusd_serusb
serusb_serusb.o : serusb/serusb.c
	$(COMPILE.c) -o serusb_serusb.o serusb/serusb.c
serusb_interrupt.o : serusb/interrupt.c
	$(COMPILE.c) -o serusb_interrupt.o serusb/interrupt.c
serusboui.o : serusboui.c
serusboui.c : serusb/serusb.oui subbusd.oui
	oui -o serusboui.c serusb/serusb.oui

subbusd_sc104_SOURCES = expint.c subbusd.c syscon104/sc104.c \
  syscon104/interrupt.c syscon104/hint.c syscon104/sc104init.c \
  sc104oui.c cache.c client_buf_default.c
subbusd_sc104_OBJECTS = expint.o subbusd.o sc104_sc104.o \
  sc104_interrupt.o sc104_hint.o sc104_sc104init.o sc104oui.o \
  cache.o client_buf_default.o
CPPFLAGS += -I syscon104

subbusd_sc104 : $(subbusd_sc104_OBJECTS)
	$(LINK.c) -o subbusd_sc104 $(LDADD) $(subbusd_sc104_OBJECTS)
	usemsg subbusd_sc104 sc104oui.c
	/usr/local/sbin/promote subbusd_sc104
sc104_sc104.o : syscon104/sc104.c
	$(COMPILE.c) -o sc104_sc104.o syscon104/sc104.c
sc104_interrupt.o : syscon104/interrupt.c
	$(COMPILE.c) -o sc104_interrupt.o syscon104/interrupt.c
sc104_hint.o : syscon104/hint.c
	$(COMPILE.c) -o sc104_hint.o syscon104/hint.c
sc104_sc104init.o : syscon104/sc104init.c
	$(COMPILE.c) -o sc104_sc104init.o syscon104/sc104init.c
sc104oui.c : syscon104/sc104.oui subbusd.oui
	oui -o sc104oui.c syscon104/sc104.oui

subbusd_sim_SOURCES = expint.c subbusd.c subbusd_int.h \
  sim/sim.c sim/interrupt.c sim/sim.h \
  subbusd.h simoui.c cache.c client_buf_default.c
CPPFLAGS += -I sim

subbusd_sim_OBJECTS = subbusd.o sim_sbsim.o simoui.o cache.o \
  client_buf_default.o
subbusd_sim : $(subbusd_sim_OBJECTS)
	$(LINK.c) -o subbusd_sim $(LDADD) $(subbusd_sim_OBJECTS)
	usemsg subbusd_sim simoui.c
	/usr/local/sbin/promote subbusd_sim
sim_sbsim.o : sim/sbsim.c
	$(COMPILE.c) -o sim_sbsim.o sim/sbsim.c
simoui.o : simoui.c
simoui.c : sim/sbsim.oui subbusd.oui
	oui -o simoui.c sim/sbsim.oui

subbusd_CAN_SOURCES = subbusd.c subbusd_int.h \
  CAN/subbusd_CAN.cc CAN/subbusd_CAN_interface.cc \
  CAN/subbusd_CAN.h CAN/subbusd_CAN_interface.h \
  CAN/timeout.h CAN/timeout.cc CAN/sb_interface.cc \
  CAN/sb_CAN.h CAN/sb_CAN.c \
  subbusd.h CANoui.cc cache.c
CPPFLAGS += -I CAN
subbusd_CAN_OBJECTS = subbusd.o cache.o CAN_sb_interface.o \
  CAN_CAN.o CAN_CAN_interface.o CAN_timeout.o CAN_sb_CAN.o CANoui.o

subbusd_CAN : $(subbusd_CAN_OBJECTS)
	$(LINK.cc) -o subbusd_CAN $(LDADD) $(subbusd_CAN_OBJECTS)
	usemsg subbusd_CAN CANoui.cc
	/usr/local/sbin/promote subbusd_CAN
CAN_CAN.o : CAN/subbusd_CAN.cc
	$(COMPILE.cc) -o CAN_CAN.o CAN/subbusd_CAN.cc
CAN_CAN_interface.o : CAN/subbusd_CAN_interface.cc
	$(COMPILE.cc) -o CAN_CAN_interface.o CAN/subbusd_CAN_interface.cc
CAN_timeout.o : CAN/timeout.cc
	$(COMPILE.cc) -o CAN_timeout.o CAN/timeout.cc
CAN_sb_interface.o : CAN/sb_interface.cc
	$(COMPILE.cc) -o CAN_sb_interface.o CAN/sb_interface.cc
CAN_sb_CAN.o : CAN/sb_CAN.c
	$(COMPILE.c) -o CAN_sb_CAN.o CAN/sb_CAN.c
CANoui.o : CANoui.cc
CANoui.cc : CAN/subbusd_slcan.oui subbusd.oui
	oui -o CANoui.cc CAN/subbusd_slcan.oui

CPPFLAGS += -I/usr/local/include 
