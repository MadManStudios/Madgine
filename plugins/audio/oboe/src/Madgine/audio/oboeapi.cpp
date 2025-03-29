#include "../oboeapilib.h"

#include "oboeapi.h"

#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Madgine/root/root.h"

#include "Generic/execution/execution.h"

#include "Madgine/resources/sender.h"

#include <oboe/Oboe.h>

METATABLE_BEGIN_BASE(Engine::Audio::OboeApi, Engine::Audio::AudioApi)
METATABLE_END(Engine::Audio::OboeApi)

UNIQUECOMPONENT(Engine::Audio::OboeApi)

namespace Engine {
namespace Audio {

    struct PlaybackState : BehaviorReceiver {
        PlaybackState(AudioLoader::Handle buffer, OboeApi *api)
            : mBuffer(std::move(buffer))
            , mApi(api)
        {
        }

        void start();

        AudioLoader::Handle mBuffer;
        OboeApi *mApi;
        const void *mBuffer;
        const void *mEnd;
    };

    template <typename Rec>
    struct PlaybackStateImpl : VirtualBehaviorState<Rec, PlaybackState> {
        using VirtualBehaviorState<Rec, PlaybackState>::VirtualBehaviorState;
    };

    struct PlaybackSender : Execution::base_sender {
        using result_type = GenericResult;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<>;

        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t, PlaybackSender &&sender, Rec &&rec)
        {
            return PlaybackStateImpl<Rec> { std::forward<Rec>(rec), std::move(sender.mBuffer), sender.mApi };
        }

        AudioLoader::Handle mBuffer;
        OboeApi *mApi;
    };

    struct OboeErrorCallback : oboe::AudioStreamErrorCallback {
    };

    struct OboeStream : oboe::AudioStreamDataCallback, std::enable_shared_from_this<OboeStream> {

        void play(PlaybackState &state)
        {
            assert(!mState);
            state.mBuffer = state.mBuffer->mBuffer.begin();
            state.mEnd = state.mBuffer->mBuffer.end();
            mState = &state;            
            mStream->requestStart();
        }

        bool isCompatible(const AudioInfo &info) const
        {
            return mStream->getChannelCount() == info.mChannels && mStream->getSampleRate() == info.mSampleRate;
        }

        oboe::DataCallbackResult onAudioReady(
            oboe::AudioStream *audioStream,
            void *audioData,
            int32_t numFrames) override
        {
            if (!Engine::Root::Root::getSingleton().taskQueue()->running()) {
                OboeApi *api = static_cast<OboeApi *>(mState->mApi);
                mState->set_done();
                mState = nullptr;
                api->reuseStream(shared_from_this());
                return oboe::DataCallbackResult::Stop;
            }

            const int16_t *source = static_cast<const int16_t *>(mBuffer);
            unsigned long count = std::min<unsigned int>(numFrames, (static_cast<const int16_t *>(mEnd) - source) / audioStream->getChannelCount());

            switch (audioStream->getFormat()) {
            case oboe::AudioFormat::I16: {
                int16_t *target = static_cast<int16_t *>(audioData);

                for (size_t i = 0; i < count; ++i) {
                    for (int i = 0; i < audioStream->getChannelCount(); ++i)
                        *target++ = *source++;
                }
                break;
            }
            case oboe::AudioFormat::Float: {
                float *target = static_cast<float *>(audioData);

                for (size_t i = 0; i < count; ++i) {
                    for (int i = 0; i < audioStream->getChannelCount(); ++i)
                        *target++ = *source++ / 32768.0f;
                }
                break;
            }
            default:
                throw 0;
            }

            mBuffer = source;
            if (count > 0) {
                return oboe::DataCallbackResult::Continue;
            } else {
                OboeApi *api = static_cast<OboeApi *>(mState->mApi);
                mState->set_value();
                mState = nullptr;
                api->reuseStream(shared_from_this());

                return oboe::DataCallbackResult::Stop;
            }
        }

        std::shared_ptr<oboe::AudioStream> mStream;

    private:
        PlaybackState *mState = nullptr;
    };

    void PlaybackState::start()
    {
        std::shared_ptr<OboeStream> stream = mApi->fetchStream(mBuffer->mInfo);
        stream->play(*this);
    }

    OboeApi::OboeApi(Root::Root &root)
        : AudioApiImpl<OboeApi>(root)
    {
        mErrorCallback = std::make_shared<OboeErrorCallback>();
    }

    OboeApi::~OboeApi()
    {
    }

    std::string_view OboeApi::key() const
    {
        return "Oboe";
    }

    Behavior OboeApi::playSound(AudioLoader::Handle buffer)
    {
        return PlaybackSender { {}, buffer, this } | Resources::with_handle(AudioLoader::Handle { buffer });
    }

    std::shared_ptr<OboeStream> OboeApi::fetchStream(const AudioInfo &info)
    {
        std::unique_lock lock { mMutex };
        auto it = std::ranges::find_if(mStreamPool, [&](const std::shared_ptr<OboeStream> &stream) { return stream->isCompatible(info); });
        if (it == mStreamPool.end()) {
            std::shared_ptr<OboeStream> stream = std::make_shared<OboeStream>();
            oboe::AudioStreamBuilder builder;
            oboe::Result result = builder.setSharingMode(oboe::SharingMode::Exclusive)
                                      ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
                                      ->setFormat(oboe::AudioFormat::I16)
                                      ->setChannelCount(info.mChannels)
                                      ->setSampleRate(info.mSampleRate)
                                      ->setDataCallback(stream)
                                      ->setErrorCallback(mErrorCallback)
                                      // Open using a shared_ptr.
                                      ->openStream(stream->mStream);
            return stream;
        } else {
            std::shared_ptr<OboeStream> stream = std::move(*it);
            mStreamPool.erase(it);
            return stream;
        }
    }

    void OboeApi::reuseStream(std::shared_ptr<OboeStream> stream)
    {
        mStreamPool.emplace_back(std::move(stream));
    }

}
}
