#include "../portaudioapilib.h"

#include "portaudioapi.h"

#include "Generic/execution/execution.h"
#include "Generic/execution/stop_callback.h"

#include "Modules/threading/awaitables/awaitabletimepoint.h"
#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/resources/sender.h"
#include "Madgine/root/root.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "portaudio.h"

METATABLE_BEGIN_BASE(Engine::Audio::PortAudioApi, Engine::Audio::AudioApi)
METATABLE_END(Engine::Audio::PortAudioApi)

UNIQUECOMPONENT(Engine::Audio::PortAudioApi)

namespace Engine {
namespace Audio {

    struct PlaybackState : Behavior::BehaviorReceiver {
        PlaybackState(AudioLoader::Handle buffer, PortAudioApi &api)
            : mBuffer(std::move(buffer))
            , mApi(api)
        {
        }

        void start();

        void stop();

        friend auto tag_invoke(Execution::visit_state_t, PlaybackState *state, auto &&visitor)
        {
            visitor(Execution::State::BeginBlock { "Play '"s + std::string { state->mBuffer.name() } + "'" });

            float progress = 0.0f;

            if (state) {
                const std::byte *start, *end, *current;
                start = static_cast<const std::byte *>(state->mBuffer->mBuffer.begin());
                end = static_cast<const std::byte *>(state->mEnd);
                current = static_cast<const std::byte *>(state->mPtr);
                progress = static_cast<float>(current - start) / (end - start);
            }

            visitor(Execution::State::Progress { progress });

            visitor(Execution::State::EndBlock {});
        }

        AudioLoader::Handle mBuffer;
        PortAudioApi &mApi;
        PortAudioStream *mStream = nullptr;

        const void *mPtr;
        const void *mEnd;

        bool mLooping = false;
    };

    template <typename Rec>
    struct PlaybackStateImpl : Behavior::VirtualBehaviorState<Rec, PlaybackState> {
        using Behavior::VirtualBehaviorState<Rec, PlaybackState>::VirtualBehaviorState;
    };

    struct PlaybackSender : Execution::base_sender {
        using result_type = KeyValueError;
        template <template <typename...> typename Tuple>
        using value_types = Tuple<>;

        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t, PlaybackSender &&sender, Rec &&rec)
        {
            return PlaybackStateImpl<Rec> { std::forward<Rec>(rec), std::move(sender.mBuffer), sender.mApi };
        }

        template <typename Rec>
        friend auto tag_invoke(Execution::connect_t, PlaybackSender &sender, Rec &&rec)
        {
            return PlaybackStateImpl<Rec> { std::forward<Rec>(rec), sender.mBuffer, sender.mApi };
        }

        AudioLoader::Handle mBuffer;
        PortAudioApi &mApi;
    };

    struct PortAudioStream {

        PortAudioStream(const AudioInfo &info, PaDeviceIndex device, const PaDeviceInfo *deviceInfo)
            : mChannels(info.mChannels)
            , mSampleRate(info.mSampleRate)
        {
            PaStreamParameters outputParameters;
            outputParameters.channelCount = info.mChannels;
            outputParameters.sampleFormat = paInt16;
            outputParameters.hostApiSpecificStreamInfo = nullptr;
            outputParameters.suggestedLatency = deviceInfo->defaultLowOutputLatency;
            outputParameters.device = device;

            PaError err = Pa_OpenStream(&mStream,
                nullptr,
                &outputParameters,
                info.mSampleRate,
                paFramesPerBufferUnspecified,
                paNoFlag,
                sCallback,
                this);
            if (err != paNoError)
                throw 0;

            err = Pa_SetStreamFinishedCallback(mStream, sFinishedCallback);
            if (err != paNoError)
                throw 0;
        }

        ~PortAudioStream()
        {
            PaError err = Pa_CloseStream(mStream);
            if (err != paNoError)
                throw 0;
            mStream = nullptr;
        }

        void play(PlaybackState &state)
        {
            assert(!mState);
            state.mPtr = state.mBuffer->mBuffer.begin();
            state.mEnd = state.mBuffer->mBuffer.end();
            mState = &state;
            assert(Pa_IsStreamStopped(mStream));
            PaError err = Pa_StartStream(mStream);
            if (err != paNoError) {
                PortAudioApi &api = state.mApi;
                mState = nullptr;
                state.set_error(KEYVALUE_UNKNOWN_ERROR() << "PortAudio Error: " << err);
                api.reuseStream(*this);
            }
        }

        PaError abort()
        {
            return Pa_AbortStream(mStream);
        }

        bool isCompatible(const AudioInfo &info) const
        {
            return mChannels == info.mChannels && mSampleRate == info.mSampleRate;
        }

    protected:
        int callback(
            void *output,
            unsigned long frameCount,
            const PaStreamCallbackTimeInfo *timeInfo,
            PaStreamCallbackFlags statusFlags)
        {
            if (Execution::get_stop_token(*mState)->stop_requested())
                return paAbort;

            int16_t *target = static_cast<int16_t *>(output);
            const int16_t *source = static_cast<const int16_t *>(mState->mPtr);

            unsigned long count = std::min<unsigned int>(frameCount, (static_cast<const int16_t *>(mState->mEnd) - source) / mChannels);

            for (size_t i = 0; i < count; ++i) {
                for (int i = 0; i < mChannels; ++i)
                    *target++ = *source++;
            }

            mState->mPtr = source;
            if (mState->mPtr == mState->mEnd) {
                if (mState->mLooping) {
                    mState->mPtr = mState->mBuffer->mBuffer.begin();
                    return callback(target, frameCount - count, timeInfo, statusFlags);
                } else {
                    return paComplete;
                }
            } else {
                return paContinue;
            }
        }

        void finishedCallback()
        {
            PortAudioApi &api = mState->mApi;
            if (Execution::get_stop_token(*mState)->stop_requested()) {
                mState->set_done();
            } else {
                mState->set_value();
            }
            mState = nullptr;
            api.reuseStream(*this);
        }

        static int
        sCallback(const void *input,
            void *output,
            unsigned long frameCount,
            const PaStreamCallbackTimeInfo *timeInfo,
            PaStreamCallbackFlags statusFlags,
            void *userData)
        {
            return static_cast<PortAudioStream *>(userData)->callback(output, frameCount, timeInfo, statusFlags);
        }

        static void sFinishedCallback(void *userData)
        {
            static_cast<PortAudioStream *>(userData)->finishedCallback();
        }

    private:
        PaStream *mStream = nullptr;
        PlaybackState *mState = nullptr;

        int mChannels;
        int mSampleRate;
    };

    void PlaybackState::start()
    {
        if (!mApi.state()) {
            set_done();
            return;
        }
        mStream = &mApi.fetchStream(mBuffer->mInfo);
        mStream->play(*this);
    }

    void PlaybackState::stop()
    {
    }

    PortAudioApi::PortAudioApi(Root::Root &root)
        : AudioApiImpl<PortAudioApi>(root)
    {
        root.taskQueue()->addSetupSteps([this]() { return callInit(); }, [this]() { return callFinalize(); });
    }

    PortAudioApi::~PortAudioApi()
    {
    }

    bool PortAudioApi::init()
    {
        PaError err = Pa_Initialize();
        if (err != paNoError) {
            LOG_ERROR("PortAudio error: " << Pa_GetErrorText(err));
            return false;
        }

        PaHostApiIndex apiCount = Pa_GetHostApiCount();
        if (apiCount < 0) {
            LOG_ERROR("PortAudio API count error: " << apiCount);
            PaError err = Pa_Terminate();
            if (err != paNoError)
                LOG_ERROR("PortAudio error: " << Pa_GetErrorText(err));
            return false;
        }

        PaDeviceIndex bestDeviceNum = -1;
        float bestDeviceLatency = 100.0f;

        for (PaHostApiIndex i = 0; i < apiCount; ++i) {
            const PaHostApiInfo *apiInfo = Pa_GetHostApiInfo(i);
            Log::LogDummy out { Log::MessageType::DEBUG_TYPE };
            out << "Considering Audio-API: " << apiInfo->name << " (" << apiInfo->type << ")...";
            if (apiInfo->type == PaHostApiTypeId::paWDMKS) {
                out << "Skipping WDMKS.";
                continue;
            }

            if (apiInfo->defaultOutputDevice < 0) {
                out << "no default output device.";
                continue;
            }

            const PaDeviceInfo *info = Pa_GetDeviceInfo(apiInfo->defaultOutputDevice);

            PaStreamParameters testParams;
            testParams.channelCount = 1;
            testParams.sampleFormat = paInt16;
            testParams.hostApiSpecificStreamInfo = nullptr;
            testParams.suggestedLatency = info->defaultLowOutputLatency;
            testParams.device = apiInfo->defaultOutputDevice;

            PaError result = Pa_IsFormatSupported(nullptr, &testParams, 44100);
            if (result != paFormatIsSupported) {
                out << "no matching format (" << 44100 << ", " << 1 << ")";
                continue;
            }
            result = Pa_IsFormatSupported(nullptr, &testParams, 48000);
            if (result != paFormatIsSupported) {
                out << "no matching format (" << 48000 << ", " << 1 << ")";
                continue;
            }

            testParams.channelCount = 2;

            result = Pa_IsFormatSupported(nullptr, &testParams, 44100);
            if (result != paFormatIsSupported) {
                out << "no matching format (" << 44100 << ", " << 2 << ")";
                continue;
            }
            result = Pa_IsFormatSupported(nullptr, &testParams, 48000);
            if (result != paFormatIsSupported) {
                out << "no matching format (" << 48000 << ", " << 2 << ")";
                continue;
            }

            out << "suitable";

            if (info->defaultLowOutputLatency < bestDeviceLatency) {
                bestDeviceLatency = info->defaultLowOutputLatency;
                bestDeviceNum = apiInfo->defaultOutputDevice;
            }
        }

        if (bestDeviceNum < 0) {
            LOG_ERROR("PortAudio failed to find device!");
            PaError err = Pa_Terminate();
            if (err != paNoError)
                LOG_ERROR("PortAudio error: " << Pa_GetErrorText(err));
            return false;
        }

        mDevice = bestDeviceNum;
        mDeviceInfo = Pa_GetDeviceInfo(mDevice);

        LOG_DEBUG("PortAudio picked: " << mDeviceInfo->name << " (" << Pa_GetHostApiInfo(mDeviceInfo->hostApi)->name << ")");

        // Potentially moving it to a separate thread altogether. Seems to wait on the thread.
        mRoot.taskQueue()->queue([this]() -> Threading::Task<void> {
            while (mRoot.taskQueue()->running()) {
                {
                    std::unique_lock lock { mMutex };
                    while (!mClosingStreams.empty()) {
                        PortAudioStream &stream = mClosingStreams.front();
                        lock.unlock();
                        PaError err = stream.abort();
                        assert(err == paTimedOut || err == paNoError);

                        if (err == paNoError) {
                            lock.lock();
                            mStreamPool.splice(mStreamPool.end(), mClosingStreams, mClosingStreams.begin());
                        } else {
                            break;
                        }
                    }
                }
                co_await 100ms;
            }
        });

        return true;
    }

    void PortAudioApi::finalize()
    {
        std::vector<PortAudioStream *> streams;
        std::ranges::transform(mBusyStreams, std::back_inserter(streams), [](auto &v) { return &v; });
        for (PortAudioStream *stream : streams)
            stream->abort();
        assert(mBusyStreams.empty());
        mStreamPool.clear();
        mClosingStreams.clear();
        PaError err = Pa_Terminate();
        if (err != paNoError)
            LOG_ERROR("PortAudio error: " << Pa_GetErrorText(err));
    }

    std::string_view PortAudioApi::key() const
    {
        return "PortAudio";
    }

    Behavior::Behavior PortAudioApi::playSound(AudioLoader::Handle buffer)
    {
        return PlaybackSender { {}, buffer, *this } | Resources::with_handle(AudioLoader::Handle { buffer });
    }

    PortAudioStream &PortAudioApi::fetchStream(const AudioInfo &info)
    {
        std::unique_lock lock { mMutex };
        auto it = std::ranges::find_if(mStreamPool, [&](const PortAudioStream &stream) { return stream.isCompatible(info); });
        if (it == mStreamPool.end()) {
            return mBusyStreams.emplace_back(info, mDevice, mDeviceInfo);
        } else {
            mBusyStreams.splice(mBusyStreams.end(), mStreamPool, it);
            return *it;
        }
    }

    void PortAudioApi::reuseStream(PortAudioStream &stream)
    {
        std::unique_lock lock { mMutex };
        auto it = std::ranges::find(mBusyStreams, &stream, [](auto &v) { return &v; });
        mClosingStreams.splice(mClosingStreams.end(), mBusyStreams, it);
    }

}
}
