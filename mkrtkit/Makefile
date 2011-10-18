# QNX6/mkrtkit
# This directory contains scripts for building runtime install CDs

# make install-progs
#   Just update the programs
# make install-pkgs
#   Test and install current packages
# make install
#   Do both

SBIN=mkrtkit mkrtkitarch osupdate
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
install-progs : install-sbin install-data
install : install-progs install-pkgs
# install-bin : $(BIN)
#	$(INSTALL) $(BIN) $(PREFIX)/bin
install-sbin : $(SBIN)
	mkdir -p $(PREFIX)/sbin
	$(INSTALL) $(SBIN) $(PREFIX)/sbin
install-data : $(DATA)
	mkdir -p $(PREFIX)/share/huarp
	$(INSTALL) $(DATA) $(PREFIX)/share/huarp
	$(INSTALL) usbinst.build /boot/build
	mkifs /boot/build/usbinst.build /boot/fs/usbinst.ifs
install-pkgs :
	@for pkg in $(PKGS); do \
	  [ -f pkgs/$$pkg/Makefile ] && make -C pkgs/$$pkg install; \
	  if [ -f pkgs/$$pkg/Header ]; then \
	    ./chkpkgs $$pkg || exit 1; \
	    ./install_pkg $$pkg || exit 1; \
	  fi; \
	done
