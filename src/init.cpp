#pragma once

#include <musicplayer.h>
#include <Godot.hpp>
#include <PoolArrays.hpp>
#include <Thread.hpp>
#include <Array.hpp>
#include <Node.hpp>
#include <File.hpp>
#include <SceneTree.hpp>
#include <OS.hpp>
#include <AudioStreamPlayer.hpp>
#include <Performance.hpp>
using namespace std;
using namespace godot;

class FLMusicLib : public Node
{
    GODOT_CLASS(FLMusicLib, Node);
public:
    struct MusicInfo
    {
        int track = 0;
        String resPath;
		bool loop = true;
		int loopEnd;
		int loopStart;
		int trackLength=-1;
        int currentMsec = 0;
		FileType type = FileType::VGM;

    };
	static FLMusicLib* instance;
    FLMusicLib()
    {
        instance = this;
        musicInfo= new MusicInfo();
        cout << "Music player instance created";

    }



    Ref<AudioStreamGenerator> gen;
    Ref<AudioStreamGeneratorPlayback> play;
    AudioStreamPlayer *player = nullptr;
    
	~FLMusicLib()
    {
        StopMusic();
        if(storedMusicInfo != nullptr)
        {
            delete storedMusicInfo;
        }
        if(audioThread != nullptr)
        {
            stopAudioThread =true;
			playing = false;
            audioThread->wait_to_finish();
			audioThread->free();
            audioThread = nullptr;
        }
    }
    /* _init must exist as it is called by Godot */
    void _init() { }

	unsigned int playTimeMsec = 0;
	unsigned long prevMsec = 0;
    void StartMusicThread()
    {

        audioThread = Thread::_new();
        audioThread->reference();
        audioThread->start(this,"_t",Variant(),0);
    }

    /*void _process(const real_t delta)
    {
        if(playing)
        {
            musicPlayer->HandlePlayback();
        }
    }
    */
	
    int fileLength;
	
    void _t(Object* o = nullptr)
    {
        while(!stopAudioThread)
        {
			while(playing && !stopAudioThread)
			{
				if(initMusicRestore)
				{

					break;
				}
				musicPlayer->HandlePlayback();

				auto msec =  OS::get_singleton()->get_ticks_msec();
                if(!player->get_stream_paused())
					playTimeMsec += msec - prevMsec;
				prevMsec = msec;
                if(musicPlayer->endMusic)
				{
                    //stopAudioThread = true;
					//The bindings have different things in the enum so substract 1 for now
					int ltc = int(Performance::get_singleton()->get_monitor(Performance::AUDIO_OUTPUT_LATENCY-1)*double(1000));

					Godot::print(String::num(ltc,0));
					OS::get_singleton()->delay_msec(int(double(musicPlayer->buffer_size)/double(musicPlayer->sample_rate))*1000+ltc);
					playing = false;
                    call_deferred("_MusicEnded");

				}
				else
				{
					OS::get_singleton()->delay_msec(10);
				}
			}
			if(initMusic)
			{
				_InitMusic();

			}

        }
		if(musicPlayer != nullptr)
		{
			delete musicPlayer;
			musicPlayer = nullptr;
		}
        player->stop();
        return;
    }

    void _InitMusic()
    {
        initMusic = false;
        File *f = File::_new();
        auto ErrOr = f->open(filePath,1);

        if(ErrOr != godot::Error::OK)
        {
            Godot::print("couldn't open g file\n");
            f->close();
            f->free();
			initMusicRestore = false;
            stopAudioThread = true;
            playing = false;
            return;
        }

        auto byteArray = f->get_buffer(f->get_len());
        fileLength = byteArray.size();
		auto byteData = unique_ptr<unsigned char>(new unsigned char[fileLength]);
		auto data = byteData.get();
        for(int i=0;i<fileLength;++i)
        {
			data[i] = byteArray[i];
        }

        f->close();
        f->free();

		NewPlayer();
        musicPlayer->volumeFact = vol;
        auto loaded = musicPlayer->LoadData(std::move(byteData),fileLength,fileType,track);
        if (!loaded)
        {
            playing = false;
            initMusicRestore = false;
            stopAudioThread = true;
            Godot::print_error("couldn't load file","_InitMusic","init.cpp",164);
            return;

        }
		if(fileType == FileType::VGM)
		{
			if(customGMEBufferSize)
			{
				musicPlayer->sampler->customBufferSize = gmeBufferSize;
				musicPlayer->sampler->autoBufferSize = false;
			}
		}
		if(startMsecs > 0)
		{
			musicPlayer->Seek(startMsecs);
		}
		if(musicInfo == nullptr)
		{
			musicInfo = new MusicInfo();
		}
		musicInfo->currentMsec = startMsecs;
		musicInfo->resPath = filePath;
		musicInfo->track = track;
		musicInfo->type = fileType;
		musicInfo->loop = loops;
		musicInfo->loopStart = lpStart;
		musicInfo->loopEnd = lpEnd;
		initMusicRestore = false;
		prevMsec = OS::get_singleton()->get_ticks_msec();
		playTimeMsec = startMsecs;
		musicPlayer->sampler->loop = loops;
		musicPlayer->sampler->loopPointEnd = lpEnd;
		musicPlayer->sampler->loopPointStart = lpStart;
		musicInfo->trackLength = musicPlayer->sampler->GetLengthMsec();
		musicPlayer->BeginStreaming();
        musicPlayer->HandlePlayback();
        player->play();
        playing = true;
	}

    void _MusicEnded()
    {

        /*if(audioThread != nullptr)
        {
            audioThread->wait_to_finish();
            audioThread->free();
            audioThread = nullptr;
            stopAudioThread = false;
        }*/
        StopMusic();
        emit_signal("track_ended");
		cout<<"track_ended";
    }

	void NewPlayer()
	{
		if(musicPlayer != nullptr)
		{
			delete musicPlayer;
		}
		musicPlayer = new MusicPlayer();
        musicPlayer->gen = gen;
        musicPlayer->playback = play;
		musicPlayer->player = player;
	}



	bool PlayMusic(String path,int trackNum,bool loop = true,int loopStart = -1,int loopEnd = -1,int startMsec = 0)
	{
		if(initMusic || initMusicRestore)
		{
			return false;
		}
		StopMusic();
        if(stopAudioThread || audioThread != nullptr || playing)
		{
			return false;
		}
		String mp3 = ".mp3";
		FileType type;
        String mod[5];
        int m = 0;
        mod[m++] = ".xm";
        mod[m++] = ".mod";
        mod[m++] = ".s3m";
        mod[m++] = ".it";
        mod[m++] = ".mptm";
		auto pth = path.to_lower();
		if(pth.ends_with(mp3))
		{
			type = FileType::MP3;
		}
        else
        {
            bool isMod = false;

            for(int j=0;j<m;j++)
            {
                if(pth.ends_with(mod[j]))
                {
                    isMod = true;
                    type = FileType::MOD;
                    break;

                }
            }

            if(!isMod)
            {
                auto cstr = path.utf8();
                auto ext = gme_identify_extension(cstr.get_data());

                if(ext == 0)
                {
                    Godot::print("couldn't identify extension");
                    return false;
                }

                type = FileType::VGM;

            }
        }

		File *f = File::_new();
		auto ErrOr = f->open(path,1);
		bool ok = true;
		if(ErrOr != godot::Error::OK)
		{
            Godot::print("couldn't open godot file\n");
			ok = false;
		}
		f->close();
		f->free();
		if(!ok)
		{
			return false;
		}

		fileType = type;

        startMsecs = startMsec;

		track = trackNum;
		filePath = path;
        initMusic = true;
		stopAudioThread= false;
		lpStart = loopStart;
		lpEnd = loopEnd;
		StartMusicThread();
		loops = loop;
        //_InitMusic();

	}

    void SetVolume(float vol)
    {
        player->set_volume_db(vol);
        this->vol = vol;
    }

    void StoreMusicState()
    {
		if(storedMusicInfo == nullptr)
        {
            storedMusicInfo = new MusicInfo();
        }

        storedMusicInfo->resPath = musicInfo->resPath;
        storedMusicInfo->track = musicInfo->track;
		storedMusicInfo->loopEnd = musicInfo->loopEnd;
		storedMusicInfo->loopStart = musicInfo->loopStart;
		storedMusicInfo->trackLength = musicInfo->trackLength;

		storedMusicInfo->currentMsec = playTimeMsec;
		storedMusicInfo->loop = musicInfo->loop;
		if(fileType == FileType::VGM)
		{
			if(storedMusicInfo->trackLength > 0)
			{
				storedMusicInfo->currentMsec = storedMusicInfo->currentMsec % storedMusicInfo->trackLength;

			}
			else
			{
				storedMusicInfo->currentMsec = storedMusicInfo->currentMsec % (MAX_MUSIC_SEEK_MSEC);
			}
		}
    }

    void RestoreMusicState()
    {
		if(initMusicRestore || initMusic || stopMusic || storedMusicInfo==nullptr)
        {
            return;
        }

        filePath =storedMusicInfo->resPath;
        startMsecs = storedMusicInfo->currentMsec;
        track = storedMusicInfo->track;
		PlayMusic(filePath,track,storedMusicInfo->loop,storedMusicInfo->loopStart,storedMusicInfo->loopEnd,startMsecs);
    }
	
	int GetTrackPositionMsec()
	{
		return playTimeMsec;
	}

    bool IsPlayerActive()
    {
		return false;
    }

	void StopMusic()
	{
        if(stopAudioThread )
        {
            return;
        }
		//stopMusic = true;
		initMusic = false;
		initMusicRestore =false;
		playing = false;
        stopAudioThread = true;
		if( audioThread != nullptr)
		{

			audioThread->wait_to_finish();
			audioThread->free();
			audioThread = nullptr;
		}
        stopAudioThread = false;
		player->stop();
	}
	void SetCurrentPlayTime(int msecs)
	{
		playTimeMsec = msecs;
	}
    void TogglePause()
    {
		if(playing && musicPlayer != nullptr)
		{
            player->set_stream_paused(!player->get_stream_paused());
		}
    }
	
    void _ready()
    {
        player = (AudioStreamPlayer*)get_child(0);
        gen = player->get_stream();
        gen->set_mix_rate(44100);
        //add_child(player);
        play = player->get_stream_playback();
    }

	bool IsPlaying()
	{
		return playing;
	}

	void SetGMEBufferSize(int sz)
	{
		customGMEBufferSize = true;
		gmeBufferSize = sz;
	}

	static void _register_methods()
	{
		register_method("store_music_state", &FLMusicLib::StoreMusicState);
		register_method("restore_music_state", &FLMusicLib::RestoreMusicState);
		register_method("get_position_msec", &FLMusicLib::GetTrackPositionMsec);
		register_method("play_music", &FLMusicLib::PlayMusic);
		register_method("stop_music", &FLMusicLib::StopMusic);
		register_method("toggle_pause", &FLMusicLib::TogglePause);
        register_method("_MusicEnded", &FLMusicLib::_MusicEnded);
		register_method("_t", &FLMusicLib::_t);
		register_method("_ready", &FLMusicLib::_ready);
		register_method("is_playing", &FLMusicLib::IsPlayerActive);
		register_method("set_volume", &FLMusicLib::SetVolume);
		register_method("set_gme_buffer_size", &FLMusicLib::SetGMEBufferSize);
        //register_method("_process", &FLMusicLib::_process);

		register_signal<FLMusicLib>("track_ended",Dictionary());

    }

private:
	MusicPlayer *musicPlayer = nullptr;
    float vol = 1;
	const int MAX_MUSIC_SEEK_MSEC = 60500*3;
	String filePath;
	FileType fileType;
	int track = 0;
	bool playing = false;
	int startMsecs;
	bool customGMEBufferSize = false;
	int gmeBufferSize = 0;
	MusicInfo* musicInfo = nullptr;
	MusicInfo* storedMusicInfo = nullptr;
	Thread* audioThread = nullptr;
	bool stopMusic = false;
	bool stopAudioThread = false;

	bool initMusic = false;
	bool initMusicRestore = false;
	bool loops = true;
	int lpStart = -1;
	int lpEnd = -1;

    String _name;

};

/** GDNative Initialize **/
extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *o) {
    godot::Godot::gdnative_init(o);
}

/** GDNative Terminate **/
extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *o) {
    godot::Godot::gdnative_terminate(o);
}

/** NativeScript Initialize **/
extern "C" void GDN_EXPORT godot_nativescript_init(void *handle) {
    godot::Godot::nativescript_init(handle);

	godot::register_class<FLMusicLib>();
} 
