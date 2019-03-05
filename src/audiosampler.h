#ifndef AUDIOSAMPLER_H
#define AUDIOSAMPLER_H
#pragma once
#include <memory>
#include <soundio.h>
using namespace std;
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
	virtual void WriteCallback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max) = 0;
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

	float GetVolume();


	//Fields
	int track = 0;
	int sample_rate;
	bool loop = true;
	int loopPointEnd = -1;
	int loopPointStart = -1;
	long fileSize = 0;
	unique_ptr<unsigned char> fileData;
};
#endif // AUDIOSAMPLER_H
