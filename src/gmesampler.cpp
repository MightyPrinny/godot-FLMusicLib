#include "gmesampler.h"
#include "musicplayer.h"

GMESampler::GMESampler()
{
    MusicPlayer::instance->SetBufferSize(1024);
}

GMESampler::~GMESampler()
{
	if(emu != nullptr)
	{
		gme_delete(emu);
	}
}

bool GMESampler::LoadData(unique_ptr<unsigned char> data, long size,int track)
{
	AudioSampler::LoadData(std::move(data),size,track);
	if(emu != nullptr)
	{
		gme_delete(emu);
		emu = nullptr;
	}
	auto err = gme_open_data(fileData.get(),size,&emu,sample_rate);
	if(err == 0)
	{
		err = gme_start_track(emu,track);

		return true;
	}

	std::cout << "error opening data\n";
    return false;
}

void GMESampler::FillBuffer(PoolVector2Array *buffer, int size)
{
    int16_t *smbuf = new int16_t[size*2];
    gme_play(emu,size*2,smbuf);
    if(gme_track_ended(emu))
    {
        MusicPlayer::instance->endMusic = true;
    }

    auto vol = GetVolume();
    float l = 0;
    float r = 0;
    for(int i=0;i<size;++i)
    {   
        auto il = smbuf[i*2]*vol;
        auto ir = smbuf[i*2+1]*vol;

        l = ((float) il) / (float) 32768;
        if( l > 1 ) l = 1;
        if( l < -1 ) l = -1;

        r = ((float) ir) / (float) 32768;
        if( r > 1 ) r = 1;
        if( r < -1 ) r = -1;

        buffer->set(i,Vector2(l,r));
    }
    delete[] smbuf;
}
