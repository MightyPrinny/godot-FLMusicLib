#include "openmptsampler.h"
#include "musicplayer.h"

void OpenMPTSampler::FillBuffer(PoolVector2Array *buffer, int size)
{
    auto l = new float[size];
    auto r = new float[size];
    auto count = mod->read(sample_rate,size,l,r);
    for(int i=0;i<size;++i)
    {
        buffer->set(i,Vector2(l[i],r[i]));
    }
    if(count == 0)
    {
        MusicPlayer::instance->endMusic = true;
        ended = true;
    }
    delete l;
    delete r;
}
