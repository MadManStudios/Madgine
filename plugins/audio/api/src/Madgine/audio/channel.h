#pragma once

#include "concepts.h"

namespace Engine {
namespace Audio {

    template <typename T>
    struct SourceWrapper : SourceInterface {
    };

    struct MADGINE_AUDIO_EXPORT Channel {

        ~Channel();

        bool addSource(SourceInterface *source);
        bool addSource(Source auto &&source)
        {
            return addSource(new SourceWrapper<decltype(source)> { std::forward<decltype(source)>(source) });
        }

        float mVolume = 1.0f;

    protected:
        size_t render(float *output, size_t frameCount, size_t channels, size_t sampleRate);

    private:
        struct Queue {

            bool push(SourceInterface *source);
            SourceInterface *pop();

            std::array<std::atomic<SourceInterface *>, 16> mRingBuffer;
            std::atomic<uint32_t> mWriteIndex = 0;
            std::atomic<uint32_t> mReadIndex = 0;
        };

        Queue mNewSourceQueue;
        struct SourceData {
            SourceInterface *mSource;
            size_t mOffset = 0;
        };
        std::vector<SourceData> mSources;
    };

    struct ChannelPtr {
        std::shared_ptr<Channel> mPtr;
    };

}
}