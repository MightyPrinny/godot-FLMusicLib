#ifndef AUDIOSAMPLER_H
#define AUDIOSAMPLER_H
#pragma once
#include <memory>
#include <PoolArrays.hpp>

class FLMusicLib;
class MusicPlayer;
using namespace std;
using namespace godot;
class AudioSampler
{
public:
	AudioSampler()
	{
	}
	virtual ~AudioSampler()
	{
	}

	//Methods
    void SetLibInstance(FLMusicLib* instance)
    {
        lib_instance = instance;
    }

    void SetPlayerInstance(MusicPlayer* instance)
    {
        player_instance = instance;
    }

	virtual bool LoadData(unique_ptr<unsigned char>bytes,long size,int track)
	{
		this->track = track;
		fileSize = size;
		fileData = std::move(bytes);
		return true;
	}
	virtual void Seek(int msec) = 0;
	virtual int TellMsec() = 0;
	virtual bool TrackEnded()
	{
		return !loop;
	}
	virtual int GetLengthMsec()
	{
		return -1;
	}

    virtual void FillBuffer(PoolVector2Array* buffer,int size) = 0;
	float GetVolume();

	//Fields
	int track = 0;
    int sample_rate = 44100;
	bool loop = true;
	int loopPointEnd = -1;
	int loopPointStart = -1;
	long fileSize = 0;
    bool autoBufferSize = true;
    int customBufferSize = 0;
	unique_ptr<unsigned char> fileData;
    FLMusicLib* lib_instance = nullptr;
    MusicPlayer* player_instance = nullptr;
};
#endif // AUDIOSAMPLER_H
