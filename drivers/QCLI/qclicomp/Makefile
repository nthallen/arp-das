.PHONY : install
SRCS := $(filter-out qclicomp.exe, $(wildcard qclicomp.*)) pickres.c linepos.c
qclicomp : qclicomp.exe
	cp -f qclicomp.exe qclicomp
qclicomp.exe : $(SRCS)
	@echo SRCS are $(SRCS)
	eli qclicomp.exe
install : qclicomp
	@cp -v qclicomp /usr/local/bin/qclicomp
