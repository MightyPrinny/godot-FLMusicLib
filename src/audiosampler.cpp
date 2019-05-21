#include "audiosampler.h"
#include "musicplayer.h"

float AudioSampler::GetVolume()
{
	return MusicPlayer::instance->volumeFact;
}
