#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#pragma once
class FLMusicLib;
#include <gmesampler.h>
#include <mp3sampler.h>
#include <OS.hpp>
#include <AudioStreamGenerator.hpp>
#include <AudioStreamGeneratorPlayback.hpp>
#include <PoolArrays.hpp>
#include <AudioStreamPlayer.hpp>
#include <openmptsampler.h>



using namespace godot;

enum FileType
{
	VGM,
    MP3,
    MOD,
    OGG
};



class MusicPlayer
{
public:
    MusicPlayer();
	~MusicPlayer();

public:
	bool paused = false;
    int sample_rate;
    bool LoadData(unique_ptr<unsigned char> data,int size,FileType type,int track = 0);
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

    void SetLibInstance(FLMusicLib* instance)
    {
        lib_instance = instance;
    }

	void BeginStreaming()
	{
		startMsec = godot::OS::get_singleton()->get_ticks_msec();
	}
	void TogglePause()
	{
		paused = !paused;
	}
	int GetLatency()
	{
        return int(double(sample_rate)/buffer_size);
	}

    void SetBufferSize(int size)
    {
        buffer_size = size;
        buffer.resize(size);
    }

	AudioSampler *sampler = nullptr;

    bool endMusic = false;
	float volumeFact = 1;
    bool finish_music = false;
    Ref<AudioStreamGenerator> gen;
    Ref<AudioStreamGeneratorPlayback> playback;
    FLMusicLib* lib_instance = nullptr;
    AudioStreamPlayer *player = nullptr;
    int buffer_size = 2048;
    void CleanBuffer()
    {

        for(int i=0;i<buffer_size;++i)
        {
            buffer.set(i,Vector2(0,0));
        }
    }

private:
    void FillBuffer();

    PoolVector2Array buffer = PoolVector2Array();
	bool end_audio = false;
	bool is_ok = true;
    unsigned long startMsec = 0;
};

#endif
