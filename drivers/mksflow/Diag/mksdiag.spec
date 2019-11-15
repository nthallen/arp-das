tmcbase = base.tmc

Module TMbase
Module mksflow src=mksflow.txt

SCRIPT = interact
TGTDIR = $(TGTNODE)/home/mksdiag
IGNORE = Makefile

mksdiagdisp : mksdiag.tbl
mksdiagalgo : mksdiag.tma
doit : mksdiag.doit
