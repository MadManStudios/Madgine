#pragma once

#include "Modules/threading/madgineobject.h"

#include "Madgine/audio/audioapi.h"

typedef int PaDeviceIndex;
struct PaDeviceInfo;

namespace Engine {
namespace Audio {

    struct PortAudioApi : AudioApiImpl<PortAudioApi>, Threading::MadgineObject<PortAudioApi> {

        PortAudioApi(Core::Root &root);
        ~PortAudioApi();

        bool init();
        void finalize();

        std::string_view key() const override;

    protected:
        std::shared_ptr<Channel> createChannel() override;

    private:
        PaDeviceIndex mDevice;
        const PaDeviceInfo *mDeviceInfo = nullptr;
    };

}
}