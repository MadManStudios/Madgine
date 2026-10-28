#include "../audiolib.h"

#include "audioloader.h"

#include "Generic/stream.h"

#include "Meta/reflect/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"
#include "Meta/type/storageops_impl.h"

#include "oggdecode.h"

RESOURCELOADER(Engine::Audio::AudioLoader)

namespace Engine {
namespace Audio {

    AudioLoader::AudioLoader()
        : ResourceLoader(std::vector<std::string> { ".ogg" })
    {
    }

    Stream AudioLoader::Interface::Resource::readAsStream(AudioInfo &info)
    {
        return DecodeOggFile(info, ResourceLoader::Interface::Resource::readAsStream(true));
    }

    bool AudioLoader::loadImpl(AudioBuffer &data, ResourceDataInfo &info)
    {
        Stream stream = info.resource()->readAsStream(data.mInfo);
        if (!stream)
            return false;

        size_t bufferSize = (data.mInfo.mBitsPerSample * data.mInfo.mSampleCount * data.mInfo.mChannels) / 8;

        std::unique_ptr<char[]> buffer = std::make_unique<char[]>(bufferSize);

        size_t actualSize = stream.read(buffer.get(), bufferSize);
        if (actualSize != bufferSize)
            LOG_WARNING("Mismatch loading audio-file: " << info.resource()->path() << " Diff: " << bufferSize - actualSize);

        data.mBuffer = { std::move(buffer), actualSize };

        return true;
    }

    void AudioLoader::unloadImpl(AudioBuffer &data)
    {
    }

    std::pair<size_t, bool> AudioLoader::Handle::render(float *output, size_t frameCount, size_t channels, size_t sampleRate, size_t &offset)
    {
        if (!available())
            return { 0, false };

        const AudioBuffer &buffer = **this;

        // TODO: resampling, handling different source formats

        const int16_t *data = static_cast<const int16_t *>(buffer.mBuffer.mData);

        size_t framesRemaining = buffer.mInfo.mSampleCount - offset;

        size_t renderFrames = std::min(frameCount, framesRemaining);

        for (size_t i = 0; i < renderFrames; ++i) {
            for (size_t channel = 0; channel < channels; ++channel) {
                output[i * channels + channel] = data[(offset + i) * buffer.mInfo.mChannels + (channel % buffer.mInfo.mChannels)] / 32768.0f;
            }
        }

        offset += renderFrames;

        return { renderFrames * channels, offset == buffer.mInfo.mSampleCount };
    }

}
}
