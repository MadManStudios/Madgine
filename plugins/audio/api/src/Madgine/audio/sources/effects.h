#pragma once

#include "Generic/execution/binding.h"

#include "Meta/math/pi.h"
#include "Meta/math/vector2.h"

namespace Engine {
namespace Audio {

    struct looping_t {
        template <Source Source>
        struct source_t {

            std::pair<size_t, bool> render(float *output, size_t frameCount, size_t channels, size_t sampleRate, size_t &offset)
            {
                auto [rendered, done] = mSource.render(output, frameCount, channels, sampleRate, offset);
                while (rendered > 0 && done) {
                    offset = 0;
                    std::tie(rendered, done) = mSource.render(output, frameCount, channels, sampleRate, offset);
                }
                return { rendered, done };
            }

            Source mSource;
        };

        template <Source Source>
        friend auto operator|(Source &&source, const looping_t &loop)
        {
            return source_t<Source> { std::forward<Source>(source) };
        }
    };

    inline constexpr looping_t looping;

    template <Execution::Binding<Math::Vector2> Binding>
    struct positional_t {
        template <Source Source>
        struct source_t {

            static float distanceGain(float distance, float refDistance, float maxDistance, float rolloff = 1.0f)
            {
                if (distance >= maxDistance)
                    return 0.0f; // inaudible: don't mix this voice at all
                distance = std::max(distance, refDistance); // full volume inside the reference distance
                float inverse = refDistance / (refDistance + rolloff * (distance - refDistance));

                float x = distance / maxDistance;
                float window = 1.0f - x * x * x * x; // smooth fade out, no click at maxDistance
                return inverse * window * window;
            }

            static std::pair<float, float> gainsFromPan(float pan) // pan: -1 (left) .. +1 (right), constant power
            {
                float theta = (pan + 1.0f) * (Math::PI / 4.0f);
                return { std::cos(theta), std::sin(theta) };
            }

            std::pair<size_t, bool> render(float *output, size_t frameCount, size_t channels, size_t sampleRate, size_t &offset)
            {
                float distancePan = 0.0f;
                constexpr float halfPI = Math::PI / 2.0f;

                float volume = 1.0f;

                Math::Vector2 dir;
                if (!Execution::access_binding(mBinding, [&](const Math::Vector2 &dir) {
                        volume = distanceGain(dir.length(), mRefDistance, mMaxDistance, mRolloff);
                        if (dir.x != 0.0f || dir.y != 0.0f)
                            distancePan = Math::clamp(std::atan2(dir.x, dir.y), -halfPI, halfPI) / halfPI;
                    }))
                    return { 0, true };
                auto [rendered, done] = mSource.render(output, frameCount, channels, sampleRate, offset);

                if (channels == 2) {
                    float pan = std::sin(distancePan); // -1 (left) .. +1 (right)
                    float spread = mWidth * (1.0f - std::abs(pan)); // the image narrows as the source moves to the side
                    auto [al, ar] = gainsFromPan(pan - spread); // where the source's left channel is placed
                    auto [bl, br] = gainsFromPan(pan + spread); // where the source's right channel is placed
                    float ll = volume * al;
                    float lr = volume * bl;
                    float rl = volume * ar;
                    float rr = volume * br;
                    for (size_t i = 0; i < rendered / 2; ++i) {
                        float left = output[2 * i];
                        float right = output[2 * i + 1];
                        output[2 * i] = ll * left + lr * right;
                        output[2 * i + 1] = rl * left + rr * right;
                    }
                } else {
                    for (size_t i = 0; i < rendered; ++i)
                        output[i] *= volume;
                }

                return { rendered, done };
            }

            Source mSource;
            Binding mBinding;
            float mRefDistance;
            float mMaxDistance;
            float mRolloff;
            float mWidth;
        };

        template <Source Source>
        friend auto operator|(Source &&source, positional_t &&loop)
        {
            return source_t<Source> { std::forward<Source>(source), std::forward<Binding>(loop.mBinding), loop.mRefDistance, loop.mMaxDistance, loop.mRolloff, loop.mWidth };
        }

        Binding mBinding;
        float mRefDistance;
        float mMaxDistance;
        float mRolloff;
        float mWidth;
    };

    template <Execution::Binding<Math::Vector2> Binding>
    constexpr positional_t<Binding> positional(Binding &&binding, float refDistance = 1.0f, float maxDistance = 50.0f, float rollOff = 1.0f, float width = 1.0f)
    {
        return { std::forward<Binding>(binding), refDistance, maxDistance, rollOff, width };
    }

    struct volume_t {
        template <Source Source>
        struct source_t {

            std::pair<size_t, bool> render(float *output, size_t frameCount, size_t channels, size_t sampleRate, size_t &offset)
            {
                auto [rendered, done] = mSource.render(output, frameCount, channels, sampleRate, offset);
                for (size_t i = 0; i < rendered; ++i)
                    output[i] *= mVol;
                return { rendered, done };
            }

            Source mSource;
            float mVol;
        };

        template <Source Source>
        friend auto operator|(Source &&source, const volume_t &vol)
        {
            return source_t<Source> { std::forward<Source>(source), vol.mVol };
        }

        float mVol;
    };

    constexpr volume_t volume(float vol)
    {
        return { vol };
    }

}
}