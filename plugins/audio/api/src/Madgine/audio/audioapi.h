#pragma once

#include "Madgine/behavior/behavior.h"
#include "Madgine/root/rootcomponentbase.h"
#include "Madgine/root/rootcomponentcollector.h"

#include "audioloader.h"

namespace Engine {
namespace Audio {

    ENUM_BASE(AudioResult, GenericResult);

    struct MADGINE_AUDIO_EXPORT AudioApi : Core::VirtualRootComponentBase<AudioApi> {

        AudioApi(Core::Root &root);

        Behavior::Behavior playSound(std::string_view name);
        virtual Behavior::Behavior playSound(AudioLoader::Handle buffer) = 0;
    };

    template <typename T>
    using AudioApiImpl = Reflect::VirtualScope<T, Core::RootComponentVirtualImpl<T, AudioApi>>;

    inline constexpr auto play_sound = [](AudioLoader::Handle buffer) {
        return AudioApi::getSingleton().playSound(std::move(buffer));
    };
}
}