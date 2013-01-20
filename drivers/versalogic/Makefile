LDFLAGS=-L/usr/local/lib -Wl,-rpath -Wl,/usr/local/lib
CPPFLAGS=-I/usr/local/include
LDLIBS=-lnort
CXXFLAGS=-Wall -g
INSTALL=cp -vn

DATA=exb37f_temp.tmc ebx37f_temp_col.tmc
DATA+=vl_temp.tmc vl_temp_col.tmc

all : mamba_temp
install : install-data
install-data :
	@$(INSTALL) $(DATA) $(TGTNODE)/usr/local/share/huarp
