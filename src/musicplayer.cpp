#include "musicplayer.h"
unsigned int MusicPlayer::device_count;
MusicPlayer* MusicPlayer::instance;
bool MusicPlayer::endMusic;
MusicPlayer::MusicPlayer()
{
	endMusic = false;
	MusicPlayer::instance = this;
	SoundIoBackend backend = SoundIoBackendNone;
	is_ok = true;
	soundio = soundio_create();
	if (!soundio) {
		is_ok = false;
		std::cout<<"out of memory\n";
	}
    int err = 0;

    err = soundio_connect(soundio);

	if (err) {
		is_ok = false;


	}
    sample_rate = 44100;
	soundio->on_backend_disconnect = BackendErrCallback;
	soundio->on_devices_change = DevicesChangeCallback;
	soundio_flush_events(soundio);

	auto selected_device_index = soundio_default_output_device_index(soundio);

	if (selected_device_index < 0) {
		is_ok = false;
	}
	device = soundio_get_output_device(soundio, selected_device_index);
	if (!device) {
		is_ok = false;
	}
	if (device->probe_error) {
		is_ok =  false;

	}
	if(device->ref_count == 0)
	{
		std::cout<<"waht";
		soundio_device_ref(device);
		is_ok = false;
	}
    if(soundio_device_supports_sample_rate(device,44100))
    {
        sample_rate = 44100;
    }
    else if(soundio_device_supports_sample_rate(device,48000))
    {
        sample_rate = 480000;
    }



	if(is_ok)
	{
		device_count = soundio_output_device_count(soundio);
	}
	else
	{
		cout <<"player isn't ok \n";
	}
}

MusicPlayer::~MusicPlayer()
{
	endMusic = false;
	if(outstream != nullptr)
		soundio_outstream_destroy(outstream);
	if(device != nullptr)
		soundio_device_unref(device);
	if(soundio != nullptr)
		soundio_destroy(soundio);
	if(sampler != nullptr)
		delete sampler;
}

void MusicPlayer::LoadData(unique_ptr<unsigned char> data,int size, FileType type,int track)
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
		default:
			cout <<"error\n";
		break;
	}
	sampler->sample_rate = sample_rate;
	sampler->LoadData(std::move(data),size,track);


}

void MusicPlayer::HandlePlayback()
{
	if(soundio)
		soundio_flush_events(soundio);

}

bool MusicPlayer::IsOk()
{
	return is_ok;
}

void MusicPlayer::BackendErrCallback(SoundIo *soundio, int err)
{
	std::cout << "backed err\n";
}

void MusicPlayer::DevicesChangeCallback(SoundIo *soundio)
{
	auto new_d_count = soundio_output_device_count(soundio);
	auto cddi = soundio_default_output_device_index(soundio);
	auto dd = soundio_get_output_device(soundio,cddi);
	if(new_d_count < device_count)
	{
		//if(soundiooutstream->device != SoundIoError::SoundIoErrorNone)
		std::cout <<"device disconnected\n";

	}
	if(new_d_count != device_count && instance->outstream != nullptr)
	{
		soundio_device_unref(instance->device);
		instance->device = dd;
		instance->NewOutStream();

	}
	device_count = new_d_count;
}

void MusicPlayer::StreamFailureCallback(SoundIoOutStream *soundio, int)
{
	std::cout <<"Stream failure\n";
}

void MusicPlayer::StreamUnderflowCallback(SoundIoOutStream *soundio)
{
	std::cout << "Stream underflow\n";
    if(!instance->finish_music && instance->sampler->TrackEnded())
    {
        instance->finish_music = true;
    }
}

int MusicPlayer::NewOutStream()
{
	if(outstream != nullptr)
	{
		soundio_outstream_destroy(outstream);
	}
	//outstreamStarted = false;
	outstream = soundio_outstream_create(device);
	outstream->write_callback = StreamWriteCalback;
	outstream->underflow_callback = StreamUnderflowCallback;
	outstream->name = nullptr;
    outstream->software_latency = 30;

    if(soundio_device_supports_sample_rate(device,44100))
    {
        sample_rate = 44100;
    }
    else if(soundio_device_supports_sample_rate(device,48000))
    {
        sample_rate = 48000;
    }

    outstream->sample_rate = sample_rate;
	outstream->error_callback = StreamFailureCallback;

    if(soundio_device_supports_format(device,SoundIoFormatS16LE))
    {
        format = SoundIoFormatS16LE;
    }
    else if(soundio_device_supports_format(device,SoundIoFormatS24LE))
    {
        format = SoundIoFormatS24LE;
    }
	else {
		end_audio = true;
		std::cerr<< "No suitable device format available.\n";
	}
	int err;
    outstream->format = format;
	if ((err = soundio_outstream_open(outstream))) {

		std::cerr<<"unable to open device: "<< soundio_strerror(err)<<"\n";
		queueNewStream();
		return err;
	}

	if (outstream->layout_error)
	{
		queueNewStream();
		std::cerr << "unable to start device: "<<soundio_strerror(outstream->layout_error)<<"\n";
		return err;
	}
	if ((err = soundio_outstream_start(outstream))) {
		//queueNewStream();
		std::cerr << "unable to start device: "<<soundio_strerror(err)<<"\n";
		return err;
	}

	//outstreamStarted = true;
	return 0;
}
