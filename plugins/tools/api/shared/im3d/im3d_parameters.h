#pragma once

#include "Meta/math/matrix4.h"

namespace Engine {

namespace Im3D {

    struct MeshParameters {
        Math::Matrix4 mTransform = Math::Matrix4::IDENTITY;
        std::chrono::microseconds mDuration = {};
    };

    struct Parameters {
        Math::Matrix4 mTransform = Math::Matrix4::IDENTITY;
        std::chrono::microseconds mDuration = {};
        Math::Color4 mColor = { 1, 1, 1, 1 };

        operator const MeshParameters &() const
        {
            return reinterpret_cast<const MeshParameters &>(*this);
        }
    };

    struct LineParameters {
        Math::Matrix4 mTransform = Math::Matrix4::IDENTITY;
        std::chrono::microseconds mDuration = {};
        Math::Color4 mColorA = { 1, 1, 1, 1 };
        Math::Color4 mColorB = mColorA;

        operator const MeshParameters &() const
        {
            return reinterpret_cast<const MeshParameters &>(*this);
        }
    };

    struct SphereParameters {
        Math::Matrix4 mTransform = Math::Matrix4::IDENTITY;
        std::chrono::microseconds mDuration = {};
        Math::Color4 mColor = { 1, 1, 1, 1 };
        size_t mDetail = 2;

        operator const MeshParameters &() const
        {
            return reinterpret_cast<const MeshParameters &>(*this);
        }
    };

    struct TextParameters {
        Math::Matrix4 mTransform = Math::Matrix4::IDENTITY;
        std::chrono::microseconds mDuration = {};
        Math::Color4 mColor = { 1, 1, 1, 1 };
        float mFontSize = 24;
        bool mFacingX = true;
        bool mFacingY = true;
        const char *mFontName = "OpenSans-Regular";
        Math::Vector2 mPivot = { 0.5f, 0.5f };

        operator const MeshParameters &() const
        {
            return reinterpret_cast<const MeshParameters &>(*this);
        }
    };

}

}