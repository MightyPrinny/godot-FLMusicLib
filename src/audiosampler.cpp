#include "audiosampler.h"
#include "musicplayer.h"

float AudioSampler::GetVolume()
{
    return player_instance->volumeFact;
}
