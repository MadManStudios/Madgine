#pragma once

#include "../toolscollector.h"
#include "../toolbase.h"

#include "Modules/debug/history.h"

namespace Engine {
namespace Tools {

    struct Metrics : Tool<Metrics> {
        Metrics(ImRoot &root);

        virtual void render() override;
        virtual void renderStatus() override;
        virtual void update() override;

        std::string_view key() const override;

    private:
        Debug::History<float, 120> mFramesPerSecond;

        Debug::History<float, 100> mFramesTrend;
        float mTimeBank = 0.0f;
    };

}
}

REGISTER_TYPE(Engine::Tools::Metrics)