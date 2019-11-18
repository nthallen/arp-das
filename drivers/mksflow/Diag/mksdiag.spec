tmcbase = base.tmc

Module TMbase
Module mksflow src=mksflow.txt name=$(MKSname)

SCRIPT = interact
TGTDIR = $(TGTNODE)/home/mksdiag
IGNORE = Makefile
IGNORE = $(MKSgen)
OBJ = $(MKSgen)

mksdiagdisp : mksdiag.tbl
mksdiagalgo : mksdiag.tma
doit : mksdiag.doit
%%
MKSname=MKS
MKSgen=$(MKSname).cmd $(MKSname).genui
MKSgen+=$(MKSname).tbl $(MKSname).tmc $(MKSname)_col.tmc
