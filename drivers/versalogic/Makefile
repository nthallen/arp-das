LDFLAGS=-L/usr/local/lib -Wl,-rpath -Wl,/usr/local/lib
CPPFLAGS=-I/usr/local/include
LDLIBS=-lnort
CXXFLAGS=-Wall -g
INSTALL=cp -vn

DATA=ebx37f_temp.tmc ebx37f_temp_col.tmc
DATA+=ebx11_temp.tmc
DATA+=vl_temp.tmc vl_temp_col.tmc

all : mamba_temp
install : install-data
install-data :
	@$(INSTALL) $(DATA) $(TGTNODE)/usr/local/share/huarp
clean :
	rm -f mamba_temp mamba_temp.o
