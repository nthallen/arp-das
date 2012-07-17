tmcbase = types.tmc
tmcbase = /usr/local/share/huarp/tmdf.tmc
tmcbase = /usr/local/share/huarp/cpu_usage.tmc
tmcbase = /usr/local/share/huarp/freemem.tmc
tmcbase = /usr/local/share/huarp/flttime.tmc
tmcbase = base.tmc

cmdbase = /usr/local/share/huarp/root.cmd
cmdbase = /usr/local/share/huarp/getcon.cmd

colbase = /usr/local/share/huarp/tmdf_col.tmc
colbase = /usr/local/share/huarp/cpu_usage_col.tmc
colbase = /usr/local/share/huarp/freemem_col.tmc

SCRIPT = interact
TGTDIR = $(TGTNODE)/home/DASdemo

demodisp : demo.tbl
demoalgo : demo.tma
doit : demo.doit


