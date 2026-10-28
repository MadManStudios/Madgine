#include "../audiolib.h"

#include "audioapi.h"

#include "Generic/execution/binding.h"
#include "Generic/execution/execution.h"

#include "Meta/math/pi.h"
#include "Meta/math/vector2.h"

#include "Madgine/behavior/nativebehaviorcollector.h"
#include "Madgine/root/root.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

#include "senders/audiosenders.h"

METATABLE_BEGIN(Engine::Audio::AudioApi)
METATABLE_END(Engine::Audio::AudioApi)

VIRTUALUNIQUECOMPONENTBASE(Engine::Audio::AudioApi)

namespace Engine {
namespace Audio {

    AudioApi::AudioApi(Core::Root &root)
        : VirtualRootComponentBase(root)
    {
    }

    void AudioApi::process(void *&output, OutputFormat format, const float *source, size_t frameCount, size_t channels)
    {
        switch (format) {
        case I16: {

            int16_t *target = reinterpret_cast<int16_t *>(output);

            for (size_t i = 0; i < frameCount; ++i) {
                for (int j = 0; j < channels; ++j)
                    *target++ = static_cast<int16_t>(Math::clamp(*source++, -1.0f, 1.0f) * 32768.0f);
            }
            break;
        }
        case F: {
            float *target = reinterpret_cast<float *>(output);

            for (size_t i = 0; i < frameCount; ++i) {

                for (int j = 0; j < channels; ++j)
                    *target++ = *source++;
            }
            break;
        }
        default:
            throw 0;
        }
    }

    void attach(SourceInterface *source, std::string_view channel)
    {
        AudioApi::getSingleton().addSourceToChannel(channel, source);
    }

    void attach(SourceInterface *source, ChannelPtr channel)
    {
        channel.mPtr->addSource(source);
    }

    void AudioApi::addSourceToChannel(std::string_view name, SourceInterface *source)
    {
        auto it = mChannels.find(name);
        if (it == mChannels.end()) {
            it = mChannels.try_emplace(std::string { name }, createChannel()).first;
        }
        it->second.mPtr->addSource(source);
    }

}
}

// NATIVE_BEHAVIOR(play_sound, Engine::Audio::play_sound, Engine::Behavior::InputParameter<"buffer", Engine::Audio::AudioLoader::Handle>, Engine::Behavior::InputParameter<"channel", std::string>)
