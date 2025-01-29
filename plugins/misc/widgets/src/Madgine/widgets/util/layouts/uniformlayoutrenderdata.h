#pragma once

#include "../renderdata.h"

#include "sizeconstraints.h"

namespace Engine {
namespace Widgets {

    struct UniformLayoutRenderData {
        void update(size_t elementCount, const SizeConstraints &elementConstraints, float pixelSize);
        std::pair<float, float> getElementDimensions(size_t index);
        float fullSize(size_t elementCount);

        uint32_t elementIndex(float offset);

        static SizeConstraints calculateTotalConstraints(size_t elementCount, const SizeConstraints &uniformElementConstraints);

    private:
        float mElementSize;
    };

}
}