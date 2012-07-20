.PHONY : install
SRCS := $(filter-out qclicomp.exe, $(wildcard qclicomp.*)) pickres.c linepos.c
qclicomp : qclicomp.exe
	cp -f qclicomp.exe qclicomp
	chmod u+w qclicomp
	{ ./qclicomp.exe -h 2>&1; true; } | usemsg qclicomp -
qclicomp.exe : $(SRCS)
	@echo SRCS are $(SRCS)
	eli qclicomp.exe
install : qclicomp
	@cp -v qclicomp $(TGTNODE)/usr/local/bin/qclicomp
