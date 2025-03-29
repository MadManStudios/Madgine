#pragma once

#include "Madgine/audio/audioapi.h"

#include "Modules/threading/madgineobject.h"

typedef int PaDeviceIndex;
struct PaDeviceInfo;

namespace Engine {
namespace Audio {
    
    struct PortAudioApi : AudioApiImpl<PortAudioApi>, Threading::MadgineObject<PortAudioApi> {

        PortAudioApi(Root::Root &root);
        ~PortAudioApi();

        bool init();
        void finalize();

        virtual std::string_view key() const override;

        virtual Behavior playSound(AudioLoader::Handle buffer) override;

    protected:
        friend struct PlaybackState;
        friend struct PortAudioStream;
        PortAudioStream &fetchStream(const AudioInfo &info);
        void reuseStream(PortAudioStream &stream);

    private:      
        std::mutex mMutex;
        std::list<PortAudioStream> mStreamPool;
        std::list<PortAudioStream> mBusyStreams;

        PaDeviceIndex mDevice;
        const PaDeviceInfo *mDeviceInfo = nullptr;
    };

}
}

REGISTER_TYPE(Engine::Audio::PortAudioApi)