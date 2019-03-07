GDNative library that plays mp3 and chiptune music files.
The project uses GameMuseEmu, Minimp3 and libsoundio

Notes

-Currently a lot of things are hardcoded like the sample rate(44100) and the sound format (S16LE), I think these settings work  on any hardware but I might be wrong.

-mp3 files must have a constant bitrate.


Methods

-bool PlayMusic(String path,int trackNum,bool loop = true,int loopStart = -1,int loopEnd = -1,int startMsec = 0)

-void StopMusic()

-bool IsPlaying()

-void SetVolumeMultiplier(float vol)

-void StoreMusicState()

-void RestoreMusicState()

-int GetTrackPositionMsec()

-void TogglePause()

Signals

-track_ended: emited when the music is not looping and a music track ends, timing might not be accurate(Or might not be emited at all but I haven't seen that yet) for chiptune music since the music length is unknown.
