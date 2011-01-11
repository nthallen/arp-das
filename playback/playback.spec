cmdbase = /usr/local/share/huarp/root.cmd playback.cmd
%%
binPrograms = playbacksrvr playbackclt
install : $(binPrograms)
	cp $(binPrograms) $(TGTNODE)/usr/local/bin
