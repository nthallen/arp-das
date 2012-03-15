PWD := $(shell pwd)
EXE := $(dir $(PWD))phrtg/x86/o-g/phrtg_g
.PHONY : install clean cleanish

phrtg : $(EXE)
	cp -v $(EXE) phrtg
$(EXE) : native.mk
	@echo PWD is $(PWD)
	@echo EXE is $(EXE)
	@echo MAKEPATH is $(MAKEPATH)
	@if [ ! -f $$QNX_TARGET/usr/local/include/nortlib.h ]; then \
	  echo "Must ln -s /usr/local \$$QNX_TARGET/usr/local"; \
	  exit 1; fi
	make -C../phrtg -f../phrtg-native/native.mk $(EXE)
native.mk : native.mk.in
	./mkmk native.mk.in >native.mk
install : phrtg phrtg.cmd
	@cp -vn phrtg /usr/local/bin
	@cp -vn phrtg.cmd /usr/local/share/huarp
cleanish :
	rm -f phrtg $(EXE)
clean :
	rm -f phrtg FullBuild.*
	make -C../phrtg -f../phrtg-native/native.mk clean
