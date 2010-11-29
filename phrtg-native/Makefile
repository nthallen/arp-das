PWD := $(shell pwd)
EXE := $(dir $(PWD))phrtg/x86/o-g/phrtg_g
.PHONY : install clean

phrtg : $(EXE)
	cp -v $(EXE) phrtg
$(EXE) : native.mk
	@echo PWD is $(PWD)
	@echo EXE is $(EXE)
	@if [ ! -f $$QNX_TARGET/usr/local/include/nortlib.h ]; then \
	  echo "Must ln -s /usr/local \$$QNX_TARGET/usr/local"; \
	  exit 1; fi
	make -C../phrtg -f../phrtg-native/native.mk $(EXE)
native.mk : native.mk.in
	./mkmk native.mk.in >native.mk
install : phrtg
	cp -vn phrtg /usr/local/bin
clean :
	rm -f phrtg
	make -C../phrtg -f../phrtg-native/native.mk clean
