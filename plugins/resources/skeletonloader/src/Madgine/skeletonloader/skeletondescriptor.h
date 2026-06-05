#pragma once

#include "Meta/math/matrix4.h"

namespace Engine {
namespace Render {

    struct Bone {
        std::string mName;
        Math::Matrix4 mOffsetMatrix;
        Math::Matrix4 mTTransform;
        Math::Matrix4 mPreTransform = Math::Matrix4::IDENTITY;
        IndexType<uint32_t> mParent;
        IndexType<uint32_t> mFirstChild;
    };

    struct SkeletonDescriptor {
        std::vector<Bone> mBones;
        Math::Matrix4 mMatrix;
        Math::Matrix4 mMatrix1 = Math::Matrix4::IDENTITY;
        Math::Matrix4 mMatrix2 = Math::Matrix4::IDENTITY;
    };

}
}