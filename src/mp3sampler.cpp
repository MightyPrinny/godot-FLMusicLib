#define MINIMP3_IMPLEMENTATION
#include "mp3sampler.h"
#include "init.cpp"
FLMusicLib* FLMusicLib::instance;

void MP3Sampler::WriteCallback(SoundIoOutStream *outstream, int frame_count_min, int frame_count_max)
{
	wcb = true;
	struct SoundIoChannelArea *areas;

	int err;
	int frame_count = ((int)(frame_count_max/MINIMP3_MAX_SAMPLES_PER_FRAME))*MINIMP3_MAX_SAMPLES_PER_FRAME;
	if (!frame_count)
	{
		wcb = false;
		return;
	}
	if ((err = soundio_outstream_begin_write(outstream, &areas, &frame_count))) {
		wcb = false;
		return;
	}
	const struct SoundIoChannelLayout *layout = &outstream->layout;

	int i =0;
	mp3dec_frame_info_t info;

	if(buff->size()<frame_count*2)
		buff->resize(frame_count*2);


	int bpos = 0;
	int samplesN = 0;
	int n = 0;
	int j = 0;
	auto data = fileData.get();
	j = 0;
	auto vol = GetVolume();
	do
	{
		samplesN = mp3dec_decode_frame(dec, (data+trackPos),MINIMP3_MAX_SAMPLES_PER_FRAME,frameBuff,&info);

		for(j=0;j<samplesN && i<frame_count*2;++j)
		{
			for (int channel = 0; channel < layout->channel_count; channel += 1) {
                int16_t *ptr = (int16_t*)areas[channel].ptr;

                *ptr = (int16_t)(frameBuff[j*2+channel]*vol);

				areas[channel].ptr += areas[channel].step;

				}
			i += 2;


		}

		trackPos += info.frame_bytes;
		if(loop)
		{
			if(loopPointStart < 0)
			{
				loopPointStart = 0;
			}
			if(loopPointStart >= 0)
			{
				if(loopPointEnd <=0)
				{
					loopPointEnd = PosToMsec(fileSize);
				}
				if((PosToMsec(trackPos)>=loopPointEnd) || (trackPos == fileSize-1))
				{
					trackPos = MsecToPos(loopPointStart);
					FLMusicLib::instance->SetCurrentPlayTime(loopPointStart);
				}
			}

		}

		if(trackPos >= fileSize)
		{
			trackPos = fileSize-1;
			MusicPlayer::instance->endMusic = true;
			if(i<frame_count*2)
			{
				for(;i<frame_count*2;i+=2)
				{
					for (int channel = 0; channel < layout->channel_count; channel += 1) {
						int16_t *ptr = (int16_t*)areas[channel].ptr;
						*ptr = 0;
						areas[channel].ptr += areas[channel].step;

						}
				}
			}
		}


	}while(info.frame_bytes>0 && i<frame_count*2 && trackPos != fileSize-1);
	if(kbps == 0 && info.bitrate_kbps > 0)
	{
		kbps = info.bitrate_kbps;

	}


	wcb = false;
	//std::cout <<"e\n";
	if ((err = soundio_outstream_end_write(outstream))) {
		if (err == SoundIoErrorUnderflow)
			return;
		return;
	}
}
