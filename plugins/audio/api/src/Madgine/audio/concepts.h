#pragma once

namespace Engine {
namespace Audio {

    // Returns the written value count and a bool indicating the source is done, when true

    template <typename T>
    concept Source = requires(T &t, float *output, size_t frameCount, size_t channels, size_t sampleRate, size_t &offset) {
        { t.render(output, frameCount, channels, sampleRate, offset) } -> std::same_as<std::pair<size_t, bool>>;
    };

    struct SourceInterface {
        virtual std::pair<size_t, bool> render(float *output, size_t frameCount, size_t channels, size_t sampleRate, size_t &offset) = 0;
    };

}
}