#include "../../../widgetslib.h"

#include "explicitlayoutrenderdata.h"

#include "Meta/keyvalue/metatable_impl.h"
#include "Meta/serialize/serializetable_impl.h"

METATABLE_BEGIN(Engine::Widgets::ExplicitLayoutConfig)
MEMBER(mSizingPolicy)
MEMBER(mFactor)
METATABLE_END(Engine::Widgets::ExplicitLayoutConfig)

SERIALIZETABLE_BEGIN(Engine::Widgets::ExplicitLayoutConfig)
FIELD(mSizingPolicy)
FIELD(mFactor)
SERIALIZETABLE_END(Engine::Widgets::ExplicitLayoutConfig)

namespace Engine {
namespace Widgets {

    void ExplicitLayoutRenderData::update(const std::vector<ExplicitLayoutConfig> &configs, float pixelSize, float z)
    {
        SizeConstraints fullSize;
        std::vector<SizeConstraints> elementConstraints;
        std::ranges::transform(
            configs, std::back_inserter(elementConstraints), [&](const ExplicitLayoutConfig &config) {
                SizeConstraints constraints = getConstraints(config, pixelSize, z);
                fullSize += constraints;
                return constraints;
            });

        float factor = fullSize.calculateFactor(pixelSize);
        mFullSize = 0.0f;
        mElements.resize(configs.size());
        for (size_t i = 0; i < configs.size(); ++i) {
            mElements[i].mOffset = mFullSize;
            float size = elementConstraints[i].apply(factor);
            mElements[i].mSize = size;
            mFullSize += size;
        }
    }

    std::pair<float, float> ExplicitLayoutRenderData::getElementDimensions(size_t index)
    {
        return { mElements[index].mOffset, mElements[index].mSize };
    }

    float ExplicitLayoutRenderData::fullSize()
    {
        return mFullSize;
    }

    SizeConstraints ExplicitLayoutRenderData::getConstraints(const ExplicitLayoutConfig &config, float pixelSize, float z)
    {
        switch (config.mSizingPolicy) {
        case SizingPolicy::EXPLICIT:
            return { z * config.mFactor, z * config.mFactor, z * config.mFactor };
        case SizingPolicy::EXPLICIT_RELATIVE:
            return { config.mFactor * pixelSize, config.mFactor * pixelSize, config.mFactor * pixelSize };
        case SizingPolicy::STRETCH:
            return { 40.0f * config.mFactor, 60.0f * config.mFactor, 100000.0f * config.mFactor };
        default:
            throw 0;
        }
    }

}
}
