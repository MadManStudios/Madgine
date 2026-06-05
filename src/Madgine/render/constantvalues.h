#pragma once

namespace Engine {
namespace Render {

    struct ConstantValues {
        Math::Vector3 mPos { 0, 0, 0 };
        float mW = 1;
        Math::Vector2 mPos2 { 0, 0 };
        Math::Vector3 mNormal { 0, 0, 0 };
        Math::Vector4 mColor { 1, 1, 1, 1 };
        Math::Vector2 mUV { 0, 0 };
        int mBoneIndices[4] { 0, 0, 0, 0 };
        float mBoneWeights[4] { 0.0f, 0.0f, 0.0f, 0.0f };
    };

    static constexpr uint32_t vConstantOffsets[] = {
        offsetof(ConstantValues, mPos),
        offsetof(ConstantValues, mW),
        offsetof(ConstantValues, mPos2),
        offsetof(ConstantValues, mNormal),
        offsetof(ConstantValues, mColor),
        offsetof(ConstantValues, mUV),
        offsetof(ConstantValues, mBoneIndices),
        offsetof(ConstantValues, mBoneWeights)
    };

}
}