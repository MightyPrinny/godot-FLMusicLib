#define MINIMP3_IMPLEMENTATION
#include "mp3sampler.h"
#include "init.cpp"
#include <math.h>
FLMusicLib* FLMusicLib::instance;

void MP3Sampler::WriteCallback(SoundIoOutStream *outstream, int frame_count_min, int frame_count_max)
{
	struct SoundIoChannelArea *areas;
    int err;
    int frame_count = frame_count_max; //((int)(frame_count_max/MINIMP3_MAX_SAMPLES_PER_FRAME))*MINIMP3_MAX_SAMPLES_PER_FRAME;
	if (!frame_count)
	{
		return;
	}
    if ((err = soundio_outstream_begin_write(outstream, &areas, &frame_count))) {
		wcb = false;
        std::cout<<"begin write err\n";
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

    while(!remaining->empty())
    {
        for (int channel = 0; channel < layout->channel_count; channel += 1)
        {
            int16_t *ptr = (int16_t*)areas[channel].ptr;

            *ptr = remaining->front();
            remaining->pop();
            areas[channel].ptr += areas[channel].step;

         }
         i+=1;
         if(i>=frame_count)
         {
             break;
         }
    }

	auto vol = GetVolume();
    if(i<frame_count && trackPos < fileSize-1)
    {
        do
        {
            samplesN = mp3dec_decode_frame(dec, (data+trackPos),MINIMP3_MAX_SAMPLES_PER_FRAME,frameBuff,&info);

            for(j=0;j<samplesN && i<frame_count;++j)
            {
                for (int channel = 0; channel < layout->channel_count; channel += 1) {
                    int16_t *ptr = (int16_t*)areas[channel].ptr;

                    *ptr = (int16_t)(frameBuff[j*2+channel]*vol);

                    areas[channel].ptr += areas[channel].step;

                    }
                i += 1;


            }

            trackPos += info.frame_bytes;
            if(trackPos >= fileSize )
            {
                trackPos = fileSize-1;
            }
            if(loop)
            {
                if(loopPointStart < 0)
                {
                    loopPointStart = 0;
                }
				if(loopPointEnd <=0)
				{
					loopPointEnd = PosToMsec(fileSize-1);
				}

				if((PosToMsec(trackPos)>=loopPointEnd) || (trackPos == fileSize-1))
				{
					trackPos = MsecToPos(loopPointStart);

					FLMusicLib::instance->SetCurrentPlayTime(loopPointStart);
				}


            }


            if(j<samplesN)
            {
                break;
            }


        }while(info.frame_bytes>0 && i<frame_count && trackPos != fileSize-1);
    }

    if(j<samplesN)//
    {
        while(j<samplesN)
        {
            for (int channel = 0; channel < layout->channel_count; channel += 1)
            {

                remaining->push(frameBuff[j*2+channel]*vol);

            }
            ++j;
        }
    }

    if(i<frame_count)
    {
        for(;i<frame_count;i+=1)
        {
            for (int channel = 0; channel < layout->channel_count; channel += 1) {
                int16_t *ptr = (int16_t*)areas[channel].ptr;
                *ptr = 0;
                areas[channel].ptr += areas[channel].step;

                }
        }
    }

    if(kbps == 0 && info.bitrate_kbps > 0)
	{
		kbps = info.bitrate_kbps;

	}


	//std::cout <<"e\n";
	if ((err = soundio_outstream_end_write(outstream))) {
		if (err == SoundIoErrorUnderflow)
			return;
		return;
	}
}
