# Makefile Generated Wed Nov 20 15:32:50 est 1996
MNC=idx64
HOMEDIR=/usr/local/src/das/idx64/regress
SPECFILE=idx64.spec
SRC=idx64.idx idx64.cfg types.tmc idx64.tmc idx64col.tmc
SRC+=idx64flag.tmc idx64rate.tmc idx64.fld idx64.tma idx64.doit
SRC+=idx64.edf cmdenbl.cmd idx64drv.cmd
TOOL=idx64.spec Makefile
SCRIPT=interact Experiment.config
OBJ=idx64.idx64 idx64col.c idx64.pcm tm.dac idx64coloui.c idx64col.o
OBJ+=idx64coloui.o idx64disp.tmc idx64disp.c idx64dispoui.c
OBJ+=idx64disp.o idx64dispoui.o idx64algoa.tmc idx64algo.c
OBJ+=idx64algooui.c idx64algo.o idx64algooui.o idx64extedf.oui
OBJ+=idx64ext.tmc idx64ext.c idx64extoui.c idx64ext.o idx64extoui.o
OBJ+=idx64cmd.c idx64cmd.dat idx64clt.o idx64cltoui.c idx64cltoui.o
OBJ+=idx64srvr.o idx64srvroui.c idx64srvroui.o
TGT=idx64col idx64disp idx64algo idx64doit idx64ext idx64clt
TGT+=idx64srvr
DISTRIB=idx64.fld idx64.tma types.tmc idx64.tmc idx64.pcm tm.dac
NONRCS=
RCS=$(SRC) $(TOOL) $(SCRIPT)
SOURCE=$(RCS) $(NONRCS)
OBJECT=$(OBJ) *.err
TARGET=$(TGT)
DSTRB=$(TGT) $(DISTRIB) $(SCRIPT)
include /usr/local/lib/src/appgen.mk

all : $(TGT)
distribution : $(DSTRB)
	distribute $(TGTDIR) $(DSTRB); :

idx64col : idx64col.o idx64coloui.o
	$(LINK.priv) idx64col.o idx64coloui.o
	$(OUIUSE) idx64coloui.c
	promote idx64col
idx64disp : idx64disp.o idx64dispoui.o
	$(LINK.norm) idx64disp.o idx64dispoui.o
	$(OUIUSE) idx64dispoui.c
idx64algo : idx64algo.o idx64algooui.o
	$(LINK.norm) idx64algo.o idx64algooui.o
	$(OUIUSE) idx64algooui.c
idx64doit : idx64.doit
	mkdoit2 -o idx64doit  idx64.doit && chmod a+x idx64doit
idx64ext : idx64ext.o idx64extoui.o
	$(LINK.norm) idx64ext.o idx64extoui.o
	$(OUIUSE) idx64extoui.c
idx64clt : idx64clt.o idx64cltoui.o
	$(LINK.norm) idx64clt.o idx64cltoui.o
	$(OUIUSE) idx64cltoui.c
idx64srvr : idx64srvr.o idx64srvroui.o
	$(LINK.priv) idx64srvr.o idx64srvroui.o
	$(OUIUSE) idx64srvroui.c
	promote idx64srvr
idx64col.c : types.tmc idx64.tmc idx64col.tmc
	$(TMC.col) types.tmc idx64.tmc idx64col.tmc
idx64coloui.c : $(OUIDIR)/collect.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/subbus_reqd.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/collect.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/subbus_reqd.oui $(OUIDIR)/msg.oui
idx64col.o : idx64col.c
idx64coloui.o : idx64coloui.c
idx64disp.tmc : idx64.fld
	$(FLD2DISP) idx64.fld
idx64disp.c : types.tmc idx64.tmc idx64flag.tmc idx64rate.tmc idx64disp.tmc
	$(TMC) types.tmc idx64.tmc idx64flag.tmc idx64rate.tmc idx64disp.tmc
idx64dispoui.c : $(OUIDIR)/nl_cons.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/nl_cons.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
idx64disp.o : idx64disp.c
idx64dispoui.o : idx64dispoui.c
idx64algoa.tmc : idx64.tma idx64srvr
	$(SERVER) idx64srvr
	$(TMCALGO) idx64.tma
idx64algo.c : types.tmc idx64.tmc idx64algoa.tmc
	$(TMC) types.tmc idx64.tmc idx64algoa.tmc
idx64algooui.c : $(OUIDIR)/tma.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/tma.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
idx64algo.o : idx64algo.c
idx64algooui.o : idx64algooui.c
idx64extedf.oui : idx64.edf
	$(EDF2OUI) idx64.edf
idx64ext.tmc : idx64.edf
	$(EDF2EXT) idx64.edf
idx64ext.c : types.tmc idx64.tmc idx64rate.tmc idx64ext.tmc
	$(TMC) types.tmc idx64.tmc idx64rate.tmc idx64ext.tmc
idx64extoui.c : idx64extedf.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
	$(OUI) idx64extedf.oui $(OUIDIR)/dc.oui $(OUIDIR)/msg.oui
idx64ext.o : idx64ext.c
idx64extoui.o : idx64extoui.c
idx64cmd.c : /usr/local/lib/src/root.cmd /usr/local/lib/src/idx64.cmd cmdenbl.cmd idx64drv.cmd
	$(CMDGEN) -d idx64cmd.dat /usr/local/lib/src/root.cmd /usr/local/lib/src/idx64.cmd cmdenbl.cmd idx64drv.cmd
idx64clt.o : idx64cmd.c
	$(COMPILE.clt) idx64cmd.c
idx64cltoui.c : $(OUIDIR)/kbdclt.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/kbdclt.oui $(OUIDIR)/msg.oui
idx64cltoui.o : idx64cltoui.c
idx64srvr.o : idx64cmd.c
	$(COMPILE.srvr) idx64cmd.c
idx64srvroui.c : $(OUIDIR)/server.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/subbus_opt.oui $(OUIDIR)/msg.oui
	$(OUI) $(OUIDIR)/server.oui $(OUIDIR)/seteuid.oui $(OUIDIR)/subbus_opt.oui $(OUIDIR)/msg.oui
idx64srvroui.o : idx64srvroui.c
IDX64CFG=../config/idx64cfg
idx64.tmc idx64col.tmc idx64flag.tmc idx64drv.cmd idx64.idx64 : idx64.idx
	$(IDX64CFG) idx64.idx idx64
idx64doit : idx64.fld
LDFLAGS+=-ltermlib
