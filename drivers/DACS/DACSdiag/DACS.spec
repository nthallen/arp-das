cmdbase = sws_init.cmd
cmdbase = /usr/local/share/huarp/root.cmd
cmdbase = /usr/local/share/huarp/getcon.cmd
cmdbase = /usr/local/share/huarp/idx64.cmd
cmdbase = cmdenbl.cmd
cmdbase = idx64drv.cmd
cmdbase = swstat.cmd
tmcbase = DACS.tmc idx64.tmc /usr/local/share/huarp/flttime.tmc
tmcbase = swstat.tmc

SRCDIST = swstat.h
SCRIPT = idx64.idx64 interact
TGTDIR = $(TGTNODE)/home/DACS

DACSsrvr : -lsubbus
DACScol : idx64col.tmc -lsubbus
DACSdisp : idx64flag.tmc idx64.tbl
DACSalgo : DACS.tma
doit : DACS.doit
