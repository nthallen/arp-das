# QNX6/mkrtkit
# This directory contains scripts for building runtime install CDs
BIN=
SBIN=mkrtkit mkrtkitarch
DATA=rtinstall
OSDATA=
INSTALL=cp -vn
PREFIX=$(TGTNODE)/usr/local
PKGPREFIX=$(TGTNODE)/var/huarp

PKGS := $(filter-out CVS, $(notdir $(wildcard pkgs/*)))

.PHONY : test all install install-sbin install-data install-pkgs
test :
	@echo PKGS are $(PKGS)
install : install-sbin install-data install-pkgs
install-sbin : $(SBIN)
	mkdir -p $(PREFIX)/sbin
	$(INSTALL) $(SBIN) $(PREFIX)/sbin
install-data : $(DATA)
	mkdir -p $(PREFIX)/share/huarp
	$(INSTALL) $(DATA) $(PREFIX)/share/huarp
install-pkgs :
	mkdir -p $(PKGPREFIX)/pkg
	@for pkg in $(PKGS); do \
	  mkdir $(PKGPREFIX)/pkg/$$pkg; \
	  for file in pkgs/$$pkg/*; do \
	    [ -f $$file ] && cp -v $$file $(PKGPREFIX)/pkg/$$pkg/; \
	  done; \
	done
