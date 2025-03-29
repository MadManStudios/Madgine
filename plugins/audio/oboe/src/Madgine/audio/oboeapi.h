#pragma once

#include "Madgine/audio/audioapi.h"

namespace Engine {
namespace Audio {

    struct OboeErrorCallback;
    struct OboeStream;

    struct OboeApi : AudioApiImpl<OboeApi> {

        OboeApi(Root::Root &root);
        ~OboeApi();

        virtual std::string_view key() const override;

        virtual Behavior playSound(AudioLoader::Handle buffer) override;

    protected:
        friend struct PlaybackState;
        friend struct OboeStream;
        std::shared_ptr<OboeStream> fetchStream(const AudioInfo &info);
        void reuseStream(std::shared_ptr<OboeStream> stream);

    private:
        std::shared_ptr<OboeErrorCallback> mErrorCallback;

        std::mutex mMutex;
        std::list<std::shared_ptr<OboeStream>> mStreamPool;
    };

}
}

REGISTER_TYPE(Engine::Audio::OboeApi)