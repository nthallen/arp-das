cmdbase = /usr/local/share/huarp/root.cmd
cmdbase = /usr/local/share/huarp/getcon.cmd
cmdbase = playback.cmd
OBJ = FullBuild.*
%%
binPrograms = playbacksrvr playbackclt playbackcltnc
install : $(binPrograms)
	cp -pnv $(binPrograms) $(TGTNODE)/usr/local/bin
