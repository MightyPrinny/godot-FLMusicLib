GDNative library that plays mp3 and chiptune music files.
The project uses GameMuseEmu, Minimp3 and the new Audio features from Godot 3.1.1

Notes

-mp3 files must have a constant bitrate.


Methods

-bool play_music(String path,int trackNum,bool loop = true,int loopStart = -1,int loopEnd = -1,int startMsec = 0)

-void stop_music()

-bool is_playing()

-void set_volume(float vol)

-void store_music_state()

-void restore_music_state()

-int get_track_position_msec()

-void toggle_pause()

Signals

-track_ended: emited when the music is not looping and a music track ends, timing might not be accurate(Or might not be emited at all but I haven't seen that yet) for chiptune music since the music length is unknown.
