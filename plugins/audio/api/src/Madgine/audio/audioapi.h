#pragma once

#include "Madgine/root/rootcomponentbase.h"
#include "Madgine/root/rootcomponentcollector.h"

#include "audioloader.h"
#include "channel.h"

namespace Engine {
namespace Audio {

    struct MADGINE_AUDIO_EXPORT AudioApi : Core::VirtualRootComponentBase<AudioApi> {

        AudioApi(Core::Root &root);

        void addSourceToChannel(std::string_view name, Source auto&& source) {
            addSourceToChannel(name, new SourceWrapper<decltype(source)>(std::forward<decltype(source)>(source)));
        }
        void addSourceToChannel(std::string_view name, SourceInterface *source);

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