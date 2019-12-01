#define MINIMP3_IMPLEMENTATION
#include "mp3sampler.h"
#include "init.cpp"

void MP3Sampler::FillBuffer(PoolVector2Array *buffer, int size)
{
    int frame_count = size;

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

    do
    {
        samplesN = mp3dec_decode_frame(dec, (data+trackPos),MINIMP3_MAX_SAMPLES_PER_FRAME,frameBuff,&info);
        float l;
        float r;
        for(j=0;j<samplesN && i<frame_count*2;++j)
        {
            auto il = (int16_t)(frameBuff[j*2]);
            auto ir = (int16_t)(frameBuff[j*2+1]);
            l = ((float) il) / (float) 32768;
            if( l > 1 ) l = 1;
            if( l < -1 ) l = -1;

            r = ((float) ir) / (float) 32768;
            if( r > 1 ) r = 1;
            if( r < -1 ) r = -1;

            buffer->set(i/2,Vector2(l,r));

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
                    lib_instance->SetCurrentPlayTime(loopPointStart);
                }
            }

        }

        if(trackPos >= fileSize)
        {
            trackPos = fileSize-1;
            player_instance->endMusic = true;
        }


    }while(info.frame_bytes>0 && i<frame_count*2 && trackPos != fileSize-1);
    if(kbps == 0 && info.bitrate_kbps > 0)
    {
        kbps = info.bitrate_kbps;

    }
    if(i<frame_count)
    {
        for(;i<frame_count;++i)
        {
            buffer->set(i,Vector2(0,0));
        }
    }
}
