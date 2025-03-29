#pragma once

#include "../renderdata.h"

#include "sizeconstraints.h"

namespace Engine {
namespace Widgets {

    ENUM(SizingPolicy,
        EXPLICIT,
        EXPLICIT_RELATIVE,
        STRETCH);

    struct ExplicitLayoutConfig {
        SizingPolicy mSizingPolicy = SizingPolicy::STRETCH;
        float mFactor = 1.0f;
    };

    struct ExplicitLayoutRenderData {

        void update(const std::vector<ExplicitLayoutConfig> &configs, float pixelSize, float z);
        std::pair<float, float> getElementDimensions(size_t index);
        float fullSize();

        static SizeConstraints getConstraints(const ExplicitLayoutConfig &config, float pixelSize, float z);

    private:
        struct ExplicitLayoutElement {
            float mOffset;
            float mSize;
        };  

        std::vector<ExplicitLayoutElement> mElements;

        float mFullSize;
    };

}
}