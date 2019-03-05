#pragma once
#include <audiosampler.h>
#include <gme.h>
#include <iostream>
#include <soundio.h>
#include <vector>
using namespace std;

class GMESampler : public AudioSampler
{
public:
	virtual ~GMESampler() override;
	virtual bool LoadData( unique_ptr<unsigned char> data, long size,int track) override;
	virtual void WriteCallback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max) override;

	virtual void Seek(int msec) override
	{
		gme_seek(emu,msec);
	}

	virtual int TellMsec() override
	{
		return gme_tell(emu);
	}
	virtual int GetLengthMsec()
	{
		gme_info_t* info;
		gme_track_info(emu,&info,0);
		auto l = info->play_length;
		gme_free_info(info);
		return l;
	}
	virtual bool TrackEnded() override
	{
		return gme_track_ended(emu);
	}
private:

	Music_Emu* emu = nullptr;
	bool playing  = true;
	unique_ptr<vector<int16_t>> buffer = unique_ptr<vector<int16_t>>(new vector<int16_t>());

};

