.PHONY : install

SRCS := Odinfile phtable.specs $(shell grep -v '^\$$' phtable.specs)

phtable : phtable.exe
	cp -f phtable.exe phtable
	chmod +w phtable
	./phtable.exe --help 2>&1 | usemsg phtable -
	chmod -w phtable
phtable.exe : $(SRCS)
	eli phtable.exe
install : phtable
	@cp -v phtable /usr/local/bin/phtable
