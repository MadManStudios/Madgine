#pragma once

#include "Madgine/resources/resourceloader.h"

#include "audiobuffer.h"

namespace Engine {
namespace Audio {

    struct AudioBuffer;

    struct MADGINE_AUDIO_EXPORT AudioLoader : Resources::ResourceLoader<AudioLoader, AudioBuffer> {

        struct Interface : ResourceLoader::Interface {
            struct MADGINE_AUDIO_EXPORT Resource : ResourceLoader::Interface::Resource {
                using ResourceLoader::Interface::Resource::Resource;

                Stream readAsStream(AudioInfo &info);
            };
        };

        AudioLoader();

        bool loadImpl(AudioBuffer &data, ResourceDataInfo &info);
        void unloadImpl(AudioBuffer &data);
    };

}
}

REGISTER_TYPE(Engine::Audio::AudioLoader)