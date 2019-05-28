#pragma once
#include "audiosampler.h"
#include <libopenmpt/libopenmpt.hpp>


class OpenMPTSampler : public AudioSampler
{
public:
    OpenMPTSampler()
    {
        customBufferSize = 2048;
        autoBufferSize = false;
    }
    virtual ~OpenMPTSampler() override
    {
        if(mod != nullptr)
        {
            delete mod;
        }

    }

    virtual void Seek(int msec) override
    {
        mod->set_position_seconds(double(msec)/double(1000.0));
    }

    virtual bool LoadData(unique_ptr<unsigned char>bytes,long size,int track) override
    {
       AudioSampler::LoadData(std::move(bytes),size,track);
       bool ok = true;
       try {
           mod = new openmpt::module(fileData.get(),size);

       } catch (openmpt::exception e) {
           Godot::print("error opening file");
           delete mod;
           mod = nullptr;

           ok = false;
       }
       if(!ok)
       {
           return false;
       }
       try {
           mod->select_subsong(track);
       } catch (openmpt::exception e) {
            ok = false;
       }
       if(!ok)
       {
           return false;
       }

       Godot::print("loaded tracker module");
       return true;

    }

    virtual int TellMsec() override
    {
		return int(mod->get_position_seconds()*1000);
    }

    virtual int GetLengthMsec() override
    {
        return int(mod->get_duration_seconds()*1000);

    }

    virtual void FillBuffer(PoolVector2Array* buffer,int size) override;
    virtual bool TrackEnded() override
    {
        return ended;
    }
    openmpt::module *mod = nullptr;
    bool ended = false;

};

