MNC=IdxC
HOMEDIR=/usr/local/src/das/idx64/config
TOOL=Makefile
SRC=idx64cfg
SRC+=idx64cfg.awk
SRC+=mydef.idx
OBJ=*.idx64 *.tmc *.cmd psr.idx
SOURCE=$(SRC) $(TOOL)
OBJECT=$(OBJ)

BIN=idx64cfg
DATA=idx64cfg.awk
BINDIR=$(TGTNODE)/usr/local/bin
DATADIR=$(TGTNODE)/usr/local/share/huarp

.PHONY : all install install-bin install-data
all :
install : install-bin install-data
install-bin : $(BIN)
	@cp -v $(BIN) $(BINDIR)
install-data : $(DATA)
	@cp -v $(DATA) $(DATADIR)

