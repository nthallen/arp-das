# QNX6/mkrtkit
# This directory contains scripts for building runtime install CDs
BIN=mkrtkit_chkconf mkrtkit_chkconfs
SBIN=mkrtkit mkrtkitarch
DATA=rtinstall
OSDATA=
INSTALL=cp -vn
PREFIX=$(TGTNODE)/usr/local
PKGPREFIX=$(TGTNODE)/var/huarp

PKGS := $(filter-out CVS, $(notdir $(wildcard pkgs/*)))

.PHONY : test all install install-bin install-sbin install-data \
 install-progs install-pkgs
test :
	@echo PKGS are $(PKGS)
install-progs : install-bin install-sbin install-data
install : install-progs install-pkgs
install-bin : $(BIN)
	$(INSTALL) $(BIN) $(PREFIX)/bin
install-sbin : $(SBIN)
	mkdir -p $(PREFIX)/sbin
	$(INSTALL) $(SBIN) $(PREFIX)/sbin
install-data : $(DATA)
	mkdir -p $(PREFIX)/share/huarp
	$(INSTALL) $(DATA) $(PREFIX)/share/huarp
install-pkgs :
	chkpkgs
	mkdir -p $(PKGPREFIX)/pkg
	@for pkg in $(PKGS); do \
	  [ -f pkgs/$$pkg/Makefile ] && make -C pkgs/$$pkg install; \
	  ./install_pkg $$pkg || exit 1; \
	done
