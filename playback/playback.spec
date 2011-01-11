cmdbase = /usr/local/share/huarp/root.cmd playback.cmd
OBJ = FullBuild.*
%%
binPrograms = playbacksrvr playbackclt playbackcltnc
install : $(binPrograms)
	cp $(binPrograms) $(TGTNODE)/usr/local/bin
