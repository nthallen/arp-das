# make include files for appgen QNX6 output.
CPPFLAGS=-I/usr/local/include -I/usr/pkg/include

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
AWK=awk > $@ -f $(LIBSRC)
FLD2DISP=$(AWK)/fld2disp.awk
EDF2EXT=$(AWK)/edf2ext.awk
PROMOTE=/usr/local/sbin/promote
SLP2CMD=$(AWK)/slp2cmd.awk
SLP2SOL=$(AWK)/slp2sol.awk
EDF2OUI=$(AWK)/edf2oui.awk
TMG2TMC=tmg2tmc > $@
CYCLE=$(AWK)/cycle.awk
TABLE=phtable > $@
NCTABLE=nctable > $@
DATAATTR=data_attr > $@
SERVER=srvr() { $$1 -V 2>/dev/null & waitfor /dev/huarp/none/cmd/server 3 || { echo $$1 failed >&2; return 1; }; }; srvr
TMAREV=tmcalgo
TMCALGO=$(TMAREV) -o $@
# SOLFMT=sft () { cat $$* >$@tmp; solfmt -o$@ $@tmp; rm $@tmp; }; sft
SOLFMT=solfmt -o$@
