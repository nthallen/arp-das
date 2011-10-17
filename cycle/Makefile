HOMEDIR=/usr/local/src/das/cycle
MNC=cycle
SUBDIRS=test cyctst
SOURCE=$(TOOL) $(SRC)

TOOL=Makefile Odinfile todo
SRC=cycle.HEAD.phi
SRC+=cycle.con
SRC+=regexpr.con
SRC+=cycle.gla
SRC+=average.lido
SRC+=avg.lido
SRC+=discard.lido
SRC+=expr.lido
SRC+=regexpr.lido
SRC+=region.lido
SRC+=trigger.lido
SRC+=cycle.ptg
SRC+=cycle.specs
SRC+=cycle.pdl

.PHONY : install clean
cycle : cycle.exe
	cp -f cycle.exe cycle
	chmod u+w cycle
cycle.exe : $(SRC)
	eli cycle.exe
install : cycle
	@cp -v cycle /usr/local/bin/cycle
clean :
	rm -f FullBuild.* cycle cycle.exe
