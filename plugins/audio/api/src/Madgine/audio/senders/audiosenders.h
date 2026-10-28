#pragma once

#include "../channel.h"

namespace Engine {
namespace Audio {

    struct AudioOptions {
        float mVolume = 1.0f;
        float mPan = 0.0f;

        bool mLooping = false;
    };

    MADGINE_AUDIO_EXPORT void attach(SourceInterface *source, std::string_view channel);
    MADGINE_AUDIO_EXPORT void attach(SourceInterface *source, ChannelPtr channel);

    template <typename Rec, typename Channel, Source Source>
    struct AudioSourceState : Execution::base_state<Rec>, SourceInterface {

        AudioSourceState(Rec &&rec, Channel &&channel, Source &&source)
            : Execution::base_state<Rec>(std::forward<Rec>(rec))
            , mChannel(std::forward<Channel>(channel))
            , mSource(std::forward<Source>(source))
        {
        }

        void start()
        {
            attach(this, mChannel);
        }

        void stop()
        {
            mStopRequested.test_and_set();
        }

        std::pair<size_t, bool> render(float *output, size_t frameCount, size_t channels, size_t sampleRate, size_t &offset)
        {
            if (mStopRequested.test()) {
                this->mRec.set_done();
                return { 0, true };
            }
            auto [rendered, done] = mSource.render(output, frameCount, channels, sampleRate, offset);
            if (done) {
                this->mRec.set_value();
            }
            return { rendered, done };
        }

        Channel mChannel;
        Source mSource;
        std::atomic_flag mStopRequested;
    };

    template <typename Channel, Source Source>
    struct AudioSourceSender {
        using is_sender = void;

        using result_type = void;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<>;

        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t, AudioSourceSender &&sender, Rec &&rec)
        {
            return AudioSourceState<Rec, Channel, Source> { std::forward<Rec>(rec), std::forward<Channel>(sender.mChannel), std::forward<Source>(sender.mSource) };
        }

        Channel mChannel;
        Source mSource;
    };

    inline auto play_source(Source auto &&source, std::string_view channel)
    {
        return AudioSourceSender<std::string, decltype(source)> { std::string { channel }, std::forward<decltype(source)>(source) };
    }

    inline auto play_source(Source auto &&source, ChannelPtr channel)
    {
        return AudioSourceSender<ChannelPtr, decltype(source)> { std::move(channel), std::forward<decltype(source)>(source) };
    }

}
}