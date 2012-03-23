.PHONY : install clean all

SRCS := Odinfile phtable.specs $(shell grep -v '^\$$' phtable.specs)
NCSRCS := Odinfile nctable.specs $(shell grep -v '^\$$' nctable.specs)

all : phtable nctable
phtable : phtable.exe
	cp -f phtable.exe phtable
	chmod +w phtable
	./phtable.exe --help 2>&1 | usemsg phtable -
	chmod -w phtable
phtable.exe : $(SRCS)
	eli phtable.exe
nctable : nctable.exe
	cp -f nctable.exe nctable
	chmod +w nctable
	./nctable.exe --help 2>&1 | usemsg nctable -
	chmod -w nctable
nctable.exe : $(NCSRCS)
	eli nctable.exe
install : all
	@cp -v phtable nctable phtbl /usr/local/bin/
clean :
	rm -f phtable phtable.exe dbphtable nctable nctable.exe FullBuild.*
	@echo "Execute 'eli -r' if you want to reset the cache"
