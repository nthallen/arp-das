PWD := $(shell pwd)
EXE := $(dir $(PWD))phrtg/x86/o-g/phrtg_g
.PHONY : install clean

phrtg : $(EXE)
	cp -v $(EXE) phrtg
$(EXE) :
	@echo PWD is $(PWD)
	@echo EXE is $(EXE)
	make -C../phrtg -f../phrtg-native/native.mk $(EXE)
install : phrtg
	cp -vn phrtg /usr/local/bin
clean :
	make -C../phrtg -f../phrtg-native/native.mk clean
