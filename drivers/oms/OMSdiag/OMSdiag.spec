tmcbase = base.tmc
cmdbase = /usr/local/share/huarp/root.cmd
cmdbase = /usr/local/share/huarp/getcon.cmd
cmdbase = /usr/local/share/huarp/OMS.cmd

TGTDIR = $(TGTNODE)/home/OMSdiag

SCRIPT = interact

OMSdiagcol :
OMSdisp : /usr/local/share/huarp/flttime.tmc OMS.tbl

doit : OMS.doit

