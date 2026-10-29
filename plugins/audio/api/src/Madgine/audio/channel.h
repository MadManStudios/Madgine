#pragma once

#include "concepts.h"

namespace Engine {
namespace Audio {

    template <Source Source>
    struct SourceWrapper : SourceInterface {
        std::pair<size_t, bool> render(float* output, size_t frameCount, size_t channels, size_t sampleRate, size_t& offset) override {
            auto [rendered, done] = mSource.render(output, frameCount, channels, sampleRate, offset);
            if (done)
                delete this;
            return { rendered, done };
        }

        Source mSource;
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
        friend struct ChannelPtr;
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

    struct MADGINE_AUDIO_EXPORT ChannelPtr {
        std::pair<size_t, bool> render(float *output, size_t frameCount, size_t channels, size_t sampleRate, size_t &offset);

        std::shared_ptr<Channel> mPtr;
    };

}
}