cmdbase = /usr/local/share/huarp/root.cmd
cmdbase = /usr/local/share/huarp/getcon.cmd
cmdbase = /usr/local/share/huarp/phrtg.cmd
cmdbase = playback.cmd
OBJ = FullBuild.*
TGTDIR = $(PWD)
%%
binPrograms = playbacksrvr playbackclt playbackcltnc
install : $(binPrograms)
	cp -pnv $(binPrograms) $(TGTNODE)/usr/local/bin
