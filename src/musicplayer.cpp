#include "musicplayer.h"

MusicPlayer::MusicPlayer()
{
	endMusic = false;
	is_ok = true;

    sample_rate = 44100;


    SetBufferSize(buffer_size);
	if(is_ok)
	{

	}
	else
	{
		cout <<"player isn't ok \n";
	}
}

MusicPlayer::~MusicPlayer()
{
	endMusic = false;
}

bool MusicPlayer::LoadData(unique_ptr<unsigned char> data,int size, FileType type,int track)
{
	if(sampler != nullptr)
	{
		delete sampler;
	}
	switch(type)
	{
		case VGM:
			sampler = new GMESampler();
		break;
		case MP3:
			sampler = new MP3Sampler();
		break;
        case MOD:
            sampler = new OpenMPTSampler();
        break;
		default:
			cout <<"error\n";
		break;
	}
	sampler->sample_rate = sample_rate;
    sampler->SetLibInstance(lib_instance);
    sampler->SetPlayerInstance(this);
    return sampler->LoadData(std::move(data),size,track);
}

void MusicPlayer::HandlePlayback()
{
    if(playback->get_frames_available()<=0)
    {
        return;
    }
    if (sampler->autoBufferSize)
    {
        SetBufferSize(playback->get_frames_available());
    }
    else if(buffer_size != sampler->customBufferSize)
    {
        SetBufferSize(sampler->customBufferSize);
    }
    if(playback->can_push_buffer(buffer_size))
    {
        FillBuffer();
        playback->push_buffer(buffer);

    }
}

bool MusicPlayer::IsOk()
{
    return is_ok;
}

void MusicPlayer::FillBuffer()
{
    sampler->FillBuffer(&buffer,buffer_size);
}

