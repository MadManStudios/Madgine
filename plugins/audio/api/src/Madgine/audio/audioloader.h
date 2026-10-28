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

        struct MADGINE_AUDIO_EXPORT Handle : ResourceLoader::Handle {

            using ResourceLoader::Handle::Handle;
            Handle(ResourceLoader::Handle handle)
                : ResourceLoader::Handle(std::move(handle))
            {
            }

            std::pair<size_t, bool> render(float *output, size_t frameCount, size_t channels, size_t sampleRate, size_t &offset);

        };

        AudioLoader();

        bool loadImpl(AudioBuffer &data, ResourceDataInfo &info);
        void unloadImpl(AudioBuffer &data);
    };

}
}