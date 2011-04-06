cmdbase = /usr/local/share/huarp/root.cmd
cmdbase = /usr/local/share/huarp/getcon.cmd
cmdbase = /usr/local/share/huarp/idx64.cmd
cmdbase = cmdenbl.cmd ana_in.cmd
cmdbase = idx64drv.cmd
cmdbase = swstat.cmd
cmdbase = dccc.cmd
cmdbase = ana_out.cmd
tmcbase = DACS.tmc idx64.tmc /usr/local/share/huarp/flttime.tmc
tmcbase = swstat.tmc ana_in.tmc ana_out.tmc
tmcbase = counts.tmc

SRCDIST = swstat.h
SCRIPT = idx64.idx64 interact dccc.dccc
TGTDIR = $(TGTNODE)/home/DACS

DACSsrvr : -lsubbus
DACScol : idx64col.tmc ana_in_cfg.tmc /usr/local/share/huarp/DACS_ID.tmc cnt_init.tmc -lsubbus
DACSdisp : idx64flag.tmc idx64.tbl ana_in.tbl digio.tmc digio.tbl ana_out.tbl counts.tbl
DACSalgo : DACS.tma
doit : DACS.doit
