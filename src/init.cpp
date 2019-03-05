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
	FLMusicLib() {instance = this;musicInfo= new MusicInfo();cout << "Music player instance created";}


    
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
				if(!musicPlayer->paused)
					playTimeMsec += msec - prevMsec;
				prevMsec = msec;
                bool end = false;
                if(musicInfo->type == VGM)
                {
                    if(musicInfo->trackLength>0 && playTimeMsec>musicInfo->trackLength)
                    {
                        end = true;
                    }
                    else if(musicPlayer->finish_music)
                    {
                        end = true;
                    }
                }
                else
                {
                    if(playTimeMsec >= musicInfo->trackLength)
                    {
                        end = true;
                    }
                }
                if(!end)
                    OS::get_singleton()->delay_msec(10);
                else {

                    stopAudioThread = true;
                    playing = false;

                    call_deferred("_MusicEnded");
                }
			}
			if(initMusic)
			{
				_InitMusic();
				playing = true;
			}

        }
		if(musicPlayer != nullptr)
		{
			delete musicPlayer;
			musicPlayer = nullptr;
		}
        return;
    }

    void _InitMusic()
    {
        initMusic = false;
        File *f = File::_new();
        auto ErrOr = f->open(filePath,1);

        if(ErrOr != godot::Error::OK)
        {
            cerr <<"couldn't open g file\n";
			initMusicRestore = false;
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
		musicPlayer->LoadData(std::move(byteData),fileLength,fileType,track);
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

	}

    void _MusicEnded()
    {
        emit_signal("track_ended");
        if(audioThread != nullptr)
        {
            audioThread->wait_to_finish();
            audioThread->free();
            audioThread = nullptr;
            stopAudioThread = false;
        }
    }

	void NewPlayer()
	{
		if(musicPlayer != nullptr)
		{
			delete musicPlayer;
		}
		musicPlayer = new MusicPlayer();
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
		auto pth = path.to_lower();
		if(pth.ends_with(mp3))
		{
			type = FileType::MP3;
		}
		else
		{
			auto cstr = path.utf8();
			auto ext = gme_identify_extension(cstr.get_data());

			if(ext == 0)
			{
				cerr << "couldn't identify extension";
				return false;
			}

			type = FileType::VGM;
		}

		File *f = File::_new();
		auto ErrOr = f->open(path,1);
		bool ok = true;
		if(ErrOr != godot::Error::OK)
		{
			cerr <<"couldn't open godot file\n";
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

    void SetVolumeMultiplier(float vol)
    {
        if(vol <0)
        {
            vol *= -1;
        }
        if(vol > 1)
        {
            vol = 1;
        }
        if(musicPlayer != nullptr)
            musicPlayer->volumeFact = vol;
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
	}
	void SetCurrentPlayTime(int msecs)
	{
		playTimeMsec = msecs;
	}
    void TogglePause()
    {
		if(playing && musicPlayer != nullptr)
		{
			musicPlayer->TogglePause();
		}
    }
	
    void _ready()
    {

    }

	bool IsPlaying()
	{
		return playing;
	}

	static void _register_methods()
	{
		register_method("StoreMusicState", &FLMusicLib::StoreMusicState);
		register_method("RestoreMusicState", &FLMusicLib::RestoreMusicState);
		register_method("GetPositionMsec", &FLMusicLib::GetTrackPositionMsec);
		register_method("PlayMusic", &FLMusicLib::PlayMusic);
		register_method("StopMusic", &FLMusicLib::StopMusic);
		register_method("TogglePause", &FLMusicLib::TogglePause);
        register_method("_MusicEnded", &FLMusicLib::_MusicEnded);
		register_method("_t", &FLMusicLib::_t);
		register_method("_ready", &FLMusicLib::_ready);
		register_method("IsPlaying", &FLMusicLib::IsPlayerActive);
		register_method("SetVolumeMultiplier", &FLMusicLib::SetVolumeMultiplier);

		register_signal<FLMusicLib>("track_ended",Dictionary());

    }

private:
	MusicPlayer *musicPlayer = nullptr;
    float vol = 1;
	const int MAX_MUSIC_SEEK_MSEC = 60500*3;
	String filePath;
	FileType fileType;
	int track = 0;
	Music_Emu* emu = nullptr;
	bool playing = false;
	int startMsecs;
	int sampleRate;
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
