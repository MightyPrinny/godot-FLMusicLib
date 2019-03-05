
#ifndef MP3SAMPLER_H
#define MP3SAMPLER_H
#pragma once
#include<audiosampler.h>
#include <iostream>
#include <fstream>

#include <soundio.h>
#include <vector>

#include <minimp3.h>

class MP3Sampler : public AudioSampler
{
public:
	virtual ~MP3Sampler() override
	{
		if(dec != nullptr)
		{
			delete dec;
		}
		delete buff;
		delete[] frameBuff;
	}
	virtual bool LoadData(unique_ptr<unsigned char>bytes,long size,int track)
	{
		this->track = track;
		fileSize = size;
		fileData = std::move(bytes);
		if(dec != nullptr)
		{
			delete dec;
		}
		dec = new mp3dec_t();
		mp3dec_init(dec);
		return true;
	}
	virtual void WriteCallback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max) override;

	virtual void Seek(int msec) override
	{

		if(kbps == 0)
		{
			CheckKbps();
		}
		if(kbps != 0)
		{
			trackPos = (unsigned long)((double)msec*((((double)kbps*(double)125)/(double)1000)));
		}
		else
		{
			cout <<"mp3: couldn't get bitrate\n";
			trackPos = 0;
		}
		if(trackPos >= fileSize-1)
		{
			trackPos = fileSize-1;
		}

	}

	void CheckKbps()
	{
		mp3dec_frame_info_t info;
		mp3dec_init(dec);
		auto data = fileData.get();

		mp3dec_decode_frame(dec, (data+0),MINIMP3_MAX_SAMPLES_PER_FRAME,frameBuff,&info);
		kbps = info.bitrate_kbps;
	}

	virtual int TellMsec() override
	{
		if(dec == nullptr)
		{
			return 0;
		}
		if(kbps == 0)
		{
			CheckKbps();
		}
		if(kbps == 0)
		{
			return trackPos;
		}
		int msec = (int)((double)trackPos/(double)kbps);
		return msec;
	}

	int PosToMsec(unsigned long pos)
	{
		if(kbps == 0)
		{
			CheckKbps();
		}
		if(kbps == 0)
		{
			return 0;
		}
		return (int)(((double)pos/(((double)kbps*(double)125)/(double)1000) ));
	}

	unsigned long MsecToPos(int msec)
	{
		if(kbps == 0)
		{
			CheckKbps();
		}
		if(kbps == 0)
		{
			return 0;
		}
		return (unsigned long)((double)msec*((((double)kbps*(double)125)/(double)1000)));
	}

	virtual int GetLengthMsec() override
	{
		return PosToMsec(fileSize);
	}

	virtual bool TrackEnded() override
	{
		return trackPos == (fileSize-1);
	}
private:
    std::vector<mp3d_sample_t >* buff = new std::vector<mp3d_sample_t >();
    mp3d_sample_t * frameBuff = new mp3d_sample_t[MINIMP3_MAX_SAMPLES_PER_FRAME*4];
	unsigned long trackPos = 0;
	mp3dec_t *dec = nullptr;
	int kbps = 0;
	int bps;
	double lt;
	bool end = false;
	bool wcb = false;


};

#endif // MP3SAMPLER_H
