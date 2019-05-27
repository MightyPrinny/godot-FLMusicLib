#ifndef AUDIOSAMPLER_H
#define AUDIOSAMPLER_H
#pragma once
#include <memory>
#include <PoolArrays.hpp>
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

    virtual void FillBuffer(PoolVector2Array* buffer,int size)
    {

    }

	float GetVolume();

	//Fields
	int track = 0;
	int sample_rate;
	bool loop = true;
	int loopPointEnd = -1;
	int loopPointStart = -1;
	long fileSize = 0;
    bool autoBufferSize = true;
    int customBufferSize = 0;
	unique_ptr<unsigned char> fileData;
};
#endif // AUDIOSAMPLER_H
