#include "../audiolib.h"

#include "audioapi.h"

#include "Madgine/root/root.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Generic/execution/execution.h"

#include "Madgine/nativebehaviorcollector.h"

METATABLE_BEGIN(Engine::Audio::AudioApi)
METATABLE_END(Engine::Audio::AudioApi)

VIRTUALUNIQUECOMPONENTBASE(Engine::Audio::AudioApi)

namespace Engine {
namespace Audio {

    AudioApi::AudioApi(Root::Root &root)
        : VirtualRootComponentBase(root)
    {
    }

    Behavior AudioApi::playSound(std::string_view name)
    {
        return playSound(AudioLoader::load(name));
    }

}
}

NATIVE_BEHAVIOR(play_sound, Engine::Audio::play_sound, Engine::InputParameter<"buffer", Engine::Audio::AudioLoader::Handle>)
