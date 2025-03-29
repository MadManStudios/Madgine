#pragma once

namespace Engine {
namespace Audio {

    struct AudioInfo {
        int mChannels;
        int mBitsPerSample;
        int mSampleRate;
        int64_t mSampleCount;
        std::chrono::microseconds mDuration;
    };

}
}