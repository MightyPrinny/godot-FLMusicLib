#pragma once
#include <soundio.h>
#include <gmesampler.h>
#include <soundio.h>
#include <mp3sampler.h>
#include <OS.hpp>
enum FileType
{
	VGM,
	MP3
};



class MusicPlayer
{
public:
    MusicPlayer();
	~MusicPlayer();

public:
	bool paused = false;
    int sample_rate;
	void LoadData(unique_ptr<unsigned char> data,int size,FileType type,int track = 0);
	void HandlePlayback();
	bool IsOk();
	void Seek(int msec)
	{
		if(sampler != nullptr)
		{
			sampler->Seek(msec);
		}
	}
	void SetVolumeFact(float v)
	{
        volumeFact = v;
	}
	void BeginStreaming()
	{
		NewOutStream();
		startMsec = godot::OS::get_singleton()->get_ticks_msec();
	}
	void TogglePause()
	{
		paused = !paused;
		soundio_outstream_pause(outstream,paused);
	}
	int GetLatency()
	{
		double l;
		soundio_outstream_get_latency(outstream,&l);
		return (l*1000);
	}
	AudioSampler *sampler = nullptr;
	static MusicPlayer *instance;
	static bool endMusic;
	float volumeFact = 1;
    SoundIoFormat format;
    bool finish_music = false;
private:
	static void BackendErrCallback(SoundIo* soundio,int err);
	static void StreamWriteCalback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max)
	{
		if(endMusic)
		{
			endMusic =false;
			MusicPlayer::instance->paused = true;
            MusicPlayer::instance->finish_music = true;
			return;
		}
		MusicPlayer::instance->WriteCallback(outstream,frame_count_min,frame_count_max);
	}
	static void DevicesChangeCallback(SoundIo* soundio);
	static void StreamFailureCallback(SoundIoOutStream* soundio,int );
	static void StreamUnderflowCallback(SoundIoOutStream* soundio);
	void queueNewStream()
	{

	}
	void WriteCallback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max)
	{
		if(sampler == nullptr)
		{
			return;
		}
		sampler->WriteCallback(outstream,frame_count_min,frame_count_max);
	}
	int NewOutStream();
	void QueueNewOutStream();

	SoundIo* soundio = nullptr;
	SoundIoDevice* device = nullptr;
    SoundIoOutStream* outstream = nullptr;

	bool end_audio = false;
	bool is_ok = true;
	unsigned long startMsec;
	static unsigned int device_count;
};

