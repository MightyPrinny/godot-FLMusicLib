#include "gmesampler.h"
#include "musicplayer.h"

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

void GMESampler::WriteCallback(SoundIoOutStream *outstream, int frame_count_min, int frame_count_max)
{

	struct SoundIoChannelArea *areas;
	int err;
	int frame_count = frame_count_max;
	if (!frame_count)
		return;
    if(MusicPlayer::instance->finish_music || TrackEnded())
    {
        return;
    }
	if ((err = soundio_outstream_begin_write(outstream, &areas, &frame_count))) {
		//fprintf(stderr, "unrecoverable stream error: %s\n", soundio_strerror(err));
		//reload_stream = true;

		return;
	}
	const struct SoundIoChannelLayout *layout = &outstream->layout;
	int16_t *smbuf = new int16_t[frame_count*2];
	gme_play(emu,frame_count*2,smbuf);
	auto vol = GetVolume();
	int frame = 0;
	for (; frame < frame_count; frame += 1) {
		int16_t sample = smbuf[frame];
		uint8_t b[2];
		b[1] = (sample & 0xFF00)>>8; // heigh byte
		b[0] = sample & 0x00FF; // low byte
		for (int channel = 0; channel < layout->channel_count; channel += 1) {
			int16_t *ptr = (int16_t*)areas[channel].ptr;
			*ptr = smbuf[frame*2+channel]*vol;
			areas[channel].ptr += areas[channel].step;
		}
	}
    if(frame<frame_count)
    {
        for(;frame<frame_count;++frame)
        {
            for (int channel = 0; channel < layout->channel_count; channel += 1) {
                int16_t *ptr = (int16_t*)areas[channel].ptr;
                *ptr = 0;
                areas[channel].ptr += areas[channel].step;

                }
        }
    }
	delete[] smbuf;
	if ((err = soundio_outstream_end_write(outstream))) {
		if (err == SoundIoErrorUnderflow)
        {

			return;
        }
		//fprintf(stderr, "unrecoverable stream error: %s\n", soundio_strerror(err));
		//queueNewStream();
		return;
	}
	//soundio_outstream_pause(outstream, want_pause);
}
