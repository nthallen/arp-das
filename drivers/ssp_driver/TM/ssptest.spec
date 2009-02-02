tmcbase = base.tmc waves1.tmc
cmdbase = /usr/local/share/huarp/root.cmd
cmdbase = ssp.cmd qcli.cmd
cmdbase = waves1.cmd

OBJ = waves1.cmd waves1.out waves1.tmc waves1.m waves1.log

TGTDIR = $(TGTNODE)/home/ssptest
SCRIPT = interact
SRCDIST = waves1.m
SRCDIST = waves1.qcli
IDISTRIB = doit Experiment.config
ssptestsrvr : 
ssptestcol : sspcol.tmc
# ssptestalgo : ssptest.tma
# sspdisp : /usr/local/share/huarp/flttime.tmc sspflags.tmc qclitypes.tmc qclibits.tmc ssp.tbl
sspdisp : /usr/local/share/huarp/flttime.tmc sspflags.tmc ssp.tbl

%%

waves1.cmd waves1.out waves1.tmc waves1.m : waves1.qcli
	qclicomp -o waves1.out -c waves1.cmd -d waves1.tmc -v waves1.log -m waves1.m waves1.qcli
CFLAGS+=-Wall -g
CXXFLAGS+=-Wall -g

