tmcbase = base.tmc idx64.tmc /usr/local/share/huarp/flttime.tmc
cmdbase = /usr/local/share/huarp/root.cmd /usr/local/share/huarp/idx64.cmd

SCRIPT = doit interact
IDISTRIB = doit
CLEANDIST
HOMEDIR = /home/idx64

idx64col : idx64col.tmc -lsubbus
idx64disp : idx64flag.tmc idx64.tbl
