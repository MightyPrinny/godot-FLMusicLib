#pragma once
#include <gmesampler.h>
#include <mp3sampler.h>
#include <OS.hpp>
#include <AudioStreamGenerator.hpp>
#include <AudioStreamGeneratorPlayback.hpp>
#include <PoolArrays.hpp>
#include <AudioStreamPlayer.hpp>

using namespace godot;

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
            CleanBuffer();
		}
	}
	void SetVolumeFact(float v)
	{
        volumeFact = v;
	}
	void BeginStreaming()
	{
		startMsec = godot::OS::get_singleton()->get_ticks_msec();
        CleanBuffer();
	}
	void TogglePause()
	{
		paused = !paused;
	}
	int GetLatency()
	{
		double l;
		return (l*1000);
	}

    void SetBufferSize(int size)
    {
        buffer_size = size;
        buffer.resize(size);
    }

	AudioSampler *sampler = nullptr;

	static MusicPlayer *instance;
	static bool endMusic;
	float volumeFact = 1;
    bool finish_music = false;
    Ref<AudioStreamGenerator> gen;
    Ref<AudioStreamGeneratorPlayback> playback;
	AudioStreamPlayer *player;

    void CleanBuffer()
    {

        for(int i=0;i<buffer_size;++i)
        {
            buffer.set(i,Vector2(0,0));
        }
        playback->clear_buffer();
    }

private:
    void FillBuffer();
    int buffer_size = 44100*2;
    PoolVector2Array buffer = PoolVector2Array();
	bool end_audio = false;
	bool is_ok = true;
	unsigned long startMsec;
	static unsigned int device_count;
};

