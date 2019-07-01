# QNX6/utils
# This directory contains miscellaneous utilities that need to be
# compiled.
BIN=unsu vmutil FullBuild man
SBIN=pkg_copy dev_copy sec_copy lib_dist pkg2pkg add_auth_keys.sh
DATA=dev_copy.sed
OSDATA=config.site
INSTALL=cp -vn
PREFIX=$(TGTNODE)/usr/local

.PHONY : all install install-sbin install-bin install-osdata install-data
all : install
install : install-bin install-osdata install-sbin install-data install-profile
install-sbin : $(SBIN)
	mkdir -p $(PREFIX)/sbin
	@$(INSTALL) $(SBIN) $(PREFIX)/sbin
install-bin : $(BIN)
	mkdir -p $(PREFIX)/bin
	@$(INSTALL) $(BIN) $(PREFIX)/bin
	@if [ ! -x /usr/bin/perl ]; then \
	  perlexe=`which perl 2>/dev/null`; \
	  if [ -n "$$perlexe" ]; then ln -s $$perlexe /usr/bin/perl; \
	  else echo "No perl found"; exit 1; fi; \
	 else true; fi
install-data : $(DATA)
	mkdir -p $(PREFIX)/share/huarp
	@$(INSTALL) $(DATA) $(PREFIX)/share/huarp
install-osdata : $(OSDATA)
	mkdir -p $(PREFIX)/share
	@$(INSTALL) $(OSDATA) $(PREFIX)/share
install-profile : pkg.sh
	[ -d /etc/profile.d ] || mkdir /etc/profile.d
	@cp -vn pkg.sh /etc/profile.d/pkg.sh
	ln -fs ./pkg.sh /etc/profile.d/pkg.ksh
#install-oui : $(OUI)
#	@mkdir -p $(PREFIX)/share/oui
#	@$(INSTALL) $(OUI) $(PREFIX)/share/oui
