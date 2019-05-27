#pragma once
#include <audiosampler.h>
#include <gme.h>
#include <iostream>
using namespace std;

class GMESampler : public AudioSampler
{
public:
    GMESampler();
    virtual ~GMESampler() override;
	virtual bool LoadData( unique_ptr<unsigned char> data, long size,int track) override;

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

    virtual void FillBuffer(PoolVector2Array* buffer,int size) override;
private:
	Music_Emu* emu = nullptr;
	bool playing  = true;

};

