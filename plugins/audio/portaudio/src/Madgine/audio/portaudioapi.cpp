#include "../portaudioapilib.h"

#include "portaudioapi.h"

#include "Generic/execution/execution.h"
#include "Generic/execution/stop_callback.h"

#include "Meta/math/pi.h"
#include "Meta/math/vector2.h"

#include "Modules/threading/awaitables/awaitabletimepoint.h"
#include "Modules/uniquecomponent/uniquecomponentcollector.h"

#include "Madgine/audio/channel.h"
#include "Madgine/resources/sender.h"
#include "Madgine/root/root.h"

#include "Meta/reflect/metatable_impl.h"

#include "portaudio.h"

METATABLE_BEGIN_BASE(Engine::Audio::PortAudioApi, Engine::Audio::AudioApi)
METATABLE_END(Engine::Audio::PortAudioApi)

UNIQUECOMPONENT(Engine::Audio::PortAudioApi)

namespace Engine {
namespace Audio {

    struct PortAudioStream : Channel {

        PortAudioStream(PaDeviceIndex device, const PaDeviceInfo *deviceInfo)
            : mChannels(2)
            , mSampleRate(44100)
        {
            PaStreamParameters outputParameters;
            outputParameters.channelCount = mChannels;
            outputParameters.sampleFormat = paInt16;
            outputParameters.hostApiSpecificStreamInfo = nullptr;
            outputParameters.suggestedLatency = deviceInfo->defaultLowOutputLatency;
            outputParameters.device = device;

            PaError err = Pa_OpenStream(&mStream,
                nullptr,
                &outputParameters,
                mSampleRate,
                paFramesPerBufferUnspecified,
                paNoFlag,
                sCallback,
                this);
            if (err != paNoError)
                throw 0;

            err = Pa_SetStreamFinishedCallback(mStream, sFinishedCallback);
            if (err != paNoError)
                throw 0;

            err = Pa_StartStream(mStream);
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

    protected:
        int callback(
            void *output,
            unsigned long frameCount,
            const PaStreamCallbackTimeInfo *timeInfo,
            PaStreamCallbackFlags statusFlags)
        {
            float buffer[2048];
            assert(frameCount * mChannels <= 2048);

            size_t renderedCount = render(buffer, frameCount, mChannels, mSampleRate);
            for (; renderedCount < frameCount * mChannels; ++renderedCount)
                buffer[renderedCount] = 0.0f;

            AudioApi::process(output, AudioApi::I16, buffer, frameCount, mChannels);

            return paContinue;
        }

        void finishedCallback()
        {
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

        int mChannels;
        int mSampleRate;
    };

    PortAudioApi::PortAudioApi(Core::Root &root)
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
            Platform::Log::LogDummy out { Platform::Log::MessageType::DEBUG_TYPE };
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
            testParams.channelCount = 2;
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

        return true;
    }

    void PortAudioApi::finalize()
    {
        mChannels.clear();

        PaError err = Pa_Terminate();
        if (err != paNoError)
            LOG_ERROR("PortAudio error: " << Pa_GetErrorText(err));
    }

    std::string_view PortAudioApi::key() const
    {
        return "PortAudio";
    }

    std::shared_ptr<Channel> PortAudioApi::createChannel()
    {
        return std::make_shared<PortAudioStream>(mDevice, mDeviceInfo);
    }

}
}
