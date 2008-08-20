# make include files for appgen QNX6 output.
CPPFLAGS=-I/usr/local/include
CXX=cc
CXXFLAGS=-lang-c++ -I/usr/local/include
AG_LDFLAGS=-L/usr/local/lib -Wl,-rpath -Wl,/usr/local/lib
LINK.norm=$(CC) $(CPPFLAGS) $(CFLAGS) $(AG_LDFLAGS) $(LDFLAGS) -o $@
LINK.priv=/bin/rm -f $@; $(LINK.norm)
TMCREV=tmc
TMC=$(TMCREV) -s -o $@ $(TMCFLAGS)
TMC.col=name=$@; $(TMC) -p -V $${name%col.cc}.pcm -c -D tm.dac $(COLFLAGS)
OUIDIR=/usr/local/share/oui
OUI=oui -o $@
OUIUSE=usemsg $@
LIBSRC=/usr/local/share/huarp
CMDGEN=cmdgen -o $@
COMPILE.clt=$(COMPILE.c) -o $@ -D CLIENT
COMPILE.srvr=$(COMPILE.c) -o $@ -D SERVER
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
DATAATTR=data_attr > $@
SERVER=srvr() { $$1 -V 2>/dev/null & waitfor /dev/huarp/none/cmd/server 3 || { echo $$1 failed >&2; return 1; }; }; srvr
TMAREV=tmcalgo
TMCALGO=$(TMAREV) -o $@
# SOLFMT=sft () { cat $$* >$@tmp; solfmt -o$@ $@tmp; rm $@tmp; }; sft
SOLFMT=solfmt -o$@
