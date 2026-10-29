#include "../audiolib.h"

#include "channel.h"

namespace Engine {
namespace Audio {

    Channel::~Channel()
    {
        assert(mSources.empty());
        assert(mNewSourceQueue.pop() == nullptr);
    }

    bool Channel::addSource(SourceInterface *source)
    {
        return mNewSourceQueue.push(source);
    }

    size_t Channel::render(float *output, size_t frameCount, size_t channels, size_t sampleRate)
    {

        while (SourceInterface *newSource = mNewSourceQueue.pop()) {
            mSources.push_back({ newSource, 0 });
        }

        for (size_t i = 0; i < frameCount * channels; ++i)
            output[i] = 0.0f;

        std::erase_if(mSources, [&](SourceData &source) {
            float buffer[2048];
            assert(frameCount * channels <= 2048);

            auto [rendered, done] = source.mSource->render(buffer, frameCount, channels, sampleRate, source.mOffset);

            for (size_t i = 0; i < rendered; ++i) {
                output[i] += buffer[i];
            }
            return done;
        });

        for (size_t i = 0; i < frameCount * channels; ++i)
            output[i] *= mVolume;

        return frameCount * channels;
    }

    bool Channel::Queue::push(SourceInterface *source)
    {
        uint32_t writeIndex = mWriteIndex.load();
        do {
            if (writeIndex - mReadIndex.load() >= 16)
                return false;
        } while (!mWriteIndex.compare_exchange_weak(writeIndex, writeIndex + 1));

        SourceInterface *prev = mRingBuffer[writeIndex % 16].exchange(source);
        assert(!prev);
        return true;
    }

    SourceInterface *Channel::Queue::pop()
    {
        uint32_t readIndex = mReadIndex.load();
        if (readIndex == mWriteIndex.load())
            return nullptr;
        SourceInterface *result = mRingBuffer[readIndex % 16].exchange(nullptr);
        if (result)
            ++mReadIndex;
        return result;
    }

    std::pair<size_t, bool> ChannelPtr::render(float *output, size_t frameCount, size_t channels, size_t sampleRate, size_t &offset)
    {
        return { mPtr->render(output, frameCount, channels, sampleRate), false };
    }

}
}