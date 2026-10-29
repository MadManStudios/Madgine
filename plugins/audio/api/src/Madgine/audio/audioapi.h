#pragma once

#include "Madgine/root/rootcomponentbase.h"
#include "Madgine/root/rootcomponentcollector.h"

#include "audioloader.h"
#include "channel.h"

namespace Engine {
namespace Audio {

    struct MADGINE_AUDIO_EXPORT AudioApi : Core::VirtualRootComponentBase<AudioApi> {

        AudioApi(Core::Root &root);

        template <Source Source>
        Reflect::Result addSourceToChannel(std::string_view name, Source &&source)
        {
            return addSourceToChannel(name, new SourceWrapper<Source>(std::forward<Source>(source)));
        }
        Reflect::Result addSourceToChannel(std::string_view name, SourceInterface *source);

        enum OutputFormat {
            I16,
            F
        };

        static void process(void *&output, OutputFormat format, const float *source, size_t frameCount, size_t channels);

    protected:
        virtual std::shared_ptr<Channel> createChannel() = 0;

        std::map<std::string, ChannelPtr, std::less<>> mChannels;
    };

    template <typename T>
    using AudioApiImpl = Reflect::VirtualScope<T, Core::RootComponentVirtualImpl<T, AudioApi>>;

}
}