include_HEADERS = subbusd.h
bin_PROGRAMS = subbusd_serusb subbusd_sc104
LDFLAGS = -L/usr/local/lib -Wl,-rpath -Wl,/usr/local/lib
CPPFLAGS = -I/usr/local/include -I. -Isyscon104
LDADD = -ltm -lnort
CFLAGS=-Wall -g

.PHONY : all install clean
all : $(bin_PROGRAMS)
clean :
	rm -f *.o $(bin_PROGRAMS) sc104oui.c serusboui.c
install :
	@cp -vn $(bin_PROGRAMS) /usr/local/bin
	@cp -vn $(include_HEADERS) /usr/local/include

subbusd_serusb_SOURCES = expint.c subbusd.c subbusd_int.h \
  serusb/serusb.c serusb/interrupt.c serusb/serusb.h \
  subbusd.h serusboui.c
subbusd_serusb_CPPFLAGS = -I serusb

subbusd_serusb_OBJECTS = expint.o subbusd.o serusb_serusb.o serusb_interrupt.o serusboui.o
subbusd_serusb : $(subbusd_serusb_OBJECTS)
	$(LINK.c) -o subbusd_serusb $(LDADD) $(subbusd_serusb_OBJECTS)
	usemsg subbusd_serusb serusboui.c
	/usr/local/sbin/promote subbusd_serusb
serusb_serusb.o : serusb/serusb.c
	$(COMPILE.c) -o serusb_serusb.o serusb/serusb.c
serusb_interrupt.o : serusb/interrupt.c
	$(COMPILE.c) -o serusb_interrupt.o serusb/interrupt.c
serusboui.o : serusboui.c
serusboui.c : serusb/serusb.oui
	oui -o serusboui.c serusb/serusb.oui

subbusd_sc104_SOURCES = expint.c subbusd.c syscon104/sc104.c \
  syscon104/interrupt.c syscon104/hint.c \
  sc104oui.c
subbusd_sc104_OBJECTS = expint.o subbusd.o sc104_sc104.o \
  sc104_interrupt.o sc104_hint.o sc104oui.o
subbusd_sc104_CPPPFLAGS = -I sc104

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
sc104oui.c : syscon104/sc104.oui
	oui -o sc104oui.c syscon104/sc104.oui
