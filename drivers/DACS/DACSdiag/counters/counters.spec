cmdbase = /usr/local/share/huarp/root.cmd
cmdbase = /usr/local/share/huarp/getcon.cmd
cmdbase = counters.cmd
tmcbase = base.tmc /usr/local/share/huarp/flttime.tmc
tmcbase = counts.tmc

SCRIPT = interact
TGTDIR = $(TGTNODE)/home/DACS/counters

counterssrvr : -lsubbus
counterscol : cnt_init.tmc -lsubbus
countersdisp : counts.tbl
countersalgo : counters.tma
doit : counters.doit
