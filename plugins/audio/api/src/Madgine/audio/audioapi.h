#pragma once

#include "Madgine/root/rootcomponentcollector.h"

#include "Madgine/root/rootcomponentbase.h"

#include "Generic/execution/virtualsender.h"

#include "audioloader.h"

#include "Madgine/behavior.h"


#include "Madgine/nativebehaviorcollector.h"

namespace Engine {
namespace Audio {

    ENUM_BASE(AudioResult, GenericResult);

    struct MADGINE_AUDIO_EXPORT AudioApi : Root::VirtualRootComponentBase<AudioApi> {

        AudioApi(Root::Root &root);
    
        Behavior playSound(std::string_view name);
        virtual Behavior playSound(AudioLoader::Handle buffer) = 0;
    };

    template <typename T>
    using AudioApiImpl = VirtualScope<T, Root::RootComponentVirtualImpl<T, AudioApi>>;

    inline constexpr auto play_sound = [](AudioLoader::Handle buffer) {
        return AudioApi::getSingleton().playSound(std::move(buffer));
    };
}
}

REGISTER_TYPE(Engine::Audio::AudioApi)

NATIVE_BEHAVIOR_DECLARATION(play_sound)