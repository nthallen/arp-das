tmcbase = types.tmc idx64.tmc
cmdbase = /usr/local/lib/src/root.cmd /usr/local/lib/src/idx64.cmd
cmdbase = cmdenbl.cmd idx64drv.cmd
SRC = idx64.idx idx64.cfg
SCRIPT = interact Experiment.config
OBJ = idx64.idx64

idx64col : idx64col.tmc
idx64disp : idx64flag.tmc idx64rate.tmc idx64.fld
idx64algo : idx64.tma
idx64doit : idx64.doit
idx64ext : idx64rate.tmc idx64.edf
%%
IDX64CFG=../config/idx64cfg
idx64.tmc idx64col.tmc idx64flag.tmc idx64drv.cmd idx64.idx64 : idx64.idx
	$(IDX64CFG) idx64.idx idx64
idx64doit : idx64.fld
LDFLAGS+=-ltermlib
