#pragma once

#include "Modules/debug/history.h"

#include "../toolbase.h"
#include "../toolscollector.h"

namespace Engine {
namespace Tools {

    struct Metrics : Tool<Metrics> {
        Metrics(ImRoot &root);

        virtual void render() override;
        virtual void update() override;

        std::string_view key() const override;

    private:
        Debug::History<float, 120> mFramesPerSecond;

        Debug::History<float, 100> mFramesTrend;
        float mTimeBank = 0.0f;
    };

}
}