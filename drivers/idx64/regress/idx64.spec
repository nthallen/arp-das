tmcbase = base.tmc idx64.tmc /usr/local/share/huarp/flttime.tmc
cmdbase = /usr/local/share/huarp/root.cmd /usr/local/share/huarp/idx64.cmd
cmdbase = idx64drv.cmd cmdenbl.cmd

SCRIPT = doit interact idx64.idx64
IDISTRIB = doit
CLEANDIST
HOMEDIR = /home/idx64
TGTDIR = $(TGTNODE)/home/idx64

idx64srvr : -lsubbus
idx64col : idx64col.tmc -lsubbus
idx64disp : idx64flag.tmc idx64.tbl
