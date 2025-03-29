#include "../../../widgetslib.h"

#include "uniformlayoutrenderdata.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

namespace Engine {
namespace Widgets {

    void UniformLayoutRenderData::update(size_t elementCount, const SizeConstraints &elementConstraints, float pixelSize)
    {
        SizeConstraints constraint = calculateTotalConstraints(elementCount, elementConstraints);
        mElementSize = elementConstraints.apply(constraint.calculateFactor(pixelSize));
    }

    std::pair<float, float> UniformLayoutRenderData::getElementDimensions(size_t index)
    {
        return { (mElementSize + 1.0f) * index, mElementSize };
    }

    float UniformLayoutRenderData::fullSize(size_t elementCount)
    {
        if (elementCount == 0)
            return 0.0f;
        return mElementSize * elementCount + (elementCount - 1) * 1.0f;
    }

    uint32_t UniformLayoutRenderData::elementIndex(float offset)
    {
        return floor((offset + 0.5f) / (mElementSize + 1.0f));
    }

    SizeConstraints UniformLayoutRenderData::calculateTotalConstraints(size_t elementCount, const SizeConstraints &uniformElementConstraints)
    {
        if (elementCount == 0)
            return { 0.0f, 0.0f, 0.0f };
        return uniformElementConstraints * elementCount + (elementCount - 1) * 1.0f;
    }

}
}
