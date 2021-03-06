cmdbase = /usr/local/share/huarp/root.cmd
cmdbase = /usr/local/share/huarp/getcon.cmd
cmdbase = cmdenbl.cmd
cmdbase = ana_in.cmd
cmdbase = swstat.cmd
cmdbase = ana_out.cmd
tmcbase = anacal.tmc /usr/local/share/huarp/flttime.tmc
tmcbase = swstat.tmc

SRCDIST = swstat.h
SCRIPT = interact
TGTDIR = $(TGTNODE)/home/DACS/anacal
OBJ = address.h

anacalsrvr : -lsubbus
anacalcol : ana_in_cfg.tmc /usr/local/share/huarp/DACS_ID.tmc -lsubbus
anacaldisp : anacal.tbl
anacalalgo : anacal.tma
anacalext : anacal.edf
doit : anacal.doit
%%
COLFLAGS=-Haddress.h
address.h : anacalcol.cc
anacalsrvr.o : address.h
