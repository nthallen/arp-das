# make include files for appgen QNX6 output.
CPPFLAGS=-I/usr/local/include -I/usr/pkg/include $(AG_CPPFLAGS)

# CXX and CXXLINK are used for compiling and linking c++ apps
# We need to use the same compiler that is used in building
# our c++ libs (tmpp, disp)
# CXX=g++ # (this is the GNU Make default
CXXLINK=$(CXX)

# If our c++ libraries (tmpp, disp) are built using cc, we can use these
# definitions:
# CXX=cc
# CXXLINK=cc -lang-c++

AG_LDFLAGS=-L/usr/local/lib -Wl,-rpath -Wl,/usr/local/lib -L/usr/pkg/lib -Wl,-rpath -Wl,/usr/pkg/lib
LINK.args=$(CPPFLAGS) $(CFLAGS) $(AG_LDFLAGS) $(LDFLAGS) -o $@
LINK.norm=$(CC) $(LINK.args)
LINK.priv=/bin/rm -f $@; $(LINK.norm)
LINK.normCXX=$(CXXLINK) $(LINK.args)
LINK.privCXX=/bin/rm -f $@; $(LINK.normCXX)
TMCREV=tmc
TMC=$(TMCREV) -s -o $@ $(TMCFLAGS)
TMC.col=name=$@; $(TMC) -p -V $${name%col.cc}.pcm -c -D tm.dac $(COLFLAGS)
OUIDIR=/usr/local/share/oui
OUI=oui -o $@
OUIUSE=usemsg $@
LIBSRC=/usr/local/share/huarp
CMDGEN=cmdgen -o $@
COMPILE.clt=$(COMPILE.cc) -o $@ -D CLIENT
COMPILE.cltnc=$(COMPILE.cc) -o $@ -D CLIENT -D NCT_INTERFACE=1
COMPILE.srvr=$(COMPILE.cc) -o $@ -D SERVER
COMPILE.sws=swscomp -o $@ -p SWData -A
COMPILE.tbl=phtable
COMPILE.tblnc=nctable
AWK=awk > $@ -f $(LIBSRC)
FLD2DISP=$(AWK)/fld2disp.awk
EDF2EXT=$(AWK)/edf2ext.awk
PROMOTE=/usr/local/sbin/promote
SLP2CMD=$(AWK)/slp2cmd.awk
SLP2SOL=$(AWK)/slp2sol.awk
EDF2OUI=$(AWK)/edf2oui.awk
TMG2TMC=tmg2tmc > $@
CYCLE=cycle > $@
TABLE=phtable > $@
NCTABLE=nctable > $@
DATAATTR=data_attr > $@
SERVER=srvr() { $$1 -V 2>/dev/null & waitfor /dev/huarp/none/cmd/server 3 || { echo $$1 failed >&2; return 1; }; }; srvr
TMAREV=tmcalgo
TMCALGO=$(TMAREV) -o $@
# SOLFMT=sft () { cat $$* >$@tmp; solfmt -o$@ $@tmp; rm $@tmp; }; sft
SOLFMT=solfmt -o$@
GENUI_ENG_DIR=../eng
GENUI2ENG=genui -d $(GENUI_ENG_DIR) -o $@ -c
GENUI2JSON=genui -o $@ -j
GENUI2CSV=genui -o $@ -c

# PHOTON Test:
# If /dev/photon is not writable, phtable will fail, but we also
# probably don't need the photon versions of display programs.
# If the generated .tmc file is empty, the executable will still
# link and run, but won't do anything useful. So if /dev/photon
# is not writable, we'll just generate an empty output file.
# Then when we make, if /dev/photon is writable and these output
# files are empty, we'll remove them, forcing a remake.
WPHOTON := $(shell [ -w /dev/photon ] && echo YES)
%tbl.tmc : %.tbl
ifeq ($(WPHOTON),YES)
	$(COMPILE.tbl) -o $@.tmp -d $@.dep.tmp $<
	mv $@.tmp $@
	{ echo -n "$@ :"; cat $@.dep.tmp; } > $@.dep
	rm $@.dep.tmp
else
	cat /usr/local/share/huarp/nulltbl.tmc > $@
endif

%tblnc.tmc : %.tbl
	$(COMPILE.tblnc) -o $@.tmp -d $@.dep.tmp $<
	mv $@.tmp $@
	{ echo -n "$@ :"; cat $@.dep.tmp; } > $@.dep
	rm $@.dep.tmp
