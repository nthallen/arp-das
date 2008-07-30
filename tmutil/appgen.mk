# make include files for appgen QNX6 output.
CFLAGS=
LDFLAGS=
LIBS=-l tmph -l tm -l nort
LINK.priv=/bin/rm -f $@; $(LINK.c) $(LIBFLAGS) -T 1 -o $@ $(LDFLAGS)
LINK.norm=$(LINK.c) $(LIBFLAGS) -o $@ $(LDFLAGS)
TMCREV=tmc
TMC=$(TMCREV) -s -o $@ $(TMCFLAGS)
TMC.col=name=$@; $(TMC) -p -V $${name%col.c}.pcm -c -D tm.dac $(COLFLAGS)
OUIDIR=/usr/local/share/oui
OUI=oui -o $@
OUIUSE=usemsg $@
LIBSRC=/usr/local/share/huarp
CMDGEN=cmdgen -o $@
COMPILE.clt=$(COMPILE.c) -fo$@ -D CLIENT
COMPILE.srvr=$(COMPILE.c) -fo$@ -D SERVER
AWK=awk > $@ -f $(LIBSRC)
FLD2DISP=$(AWK)/fld2disp.awk
EDF2EXT=$(AWK)/edf2ext.awk
SLP2CMD=$(AWK)/slp2cmd.awk
SLP2SOL=$(AWK)/slp2sol.awk
EDF2OUI=$(AWK)/edf2oui.awk
TMG2TMC=tmg2tmc > $@
CYCLE=$(AWK)/cycle.awk
TABLE=table > $@
DATAATTR=data_attr > $@
SERVER=srvr() { $$1 -vsy & namewait -p $$! cmdinterp || { $$1; return 1; }; }; srvr
TMAREV=tmcalgo
TMCALGO=$(TMAREV) -o $@
# SOLFMT=sft () { cat $$* >$@tmp; solfmt -o$@ $@tmp; rm $@tmp; }; sft
SOLFMT=solfmt -o$@