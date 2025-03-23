#pragma once

#include "Meta/math/matrix4.h"

namespace Engine {

namespace Im3D {

    struct MeshParameters {
        Engine::Matrix4 mTransform = Engine::Matrix4::IDENTITY;
        std::chrono::microseconds mDuration = {};
    };

    struct Parameters {
        Engine::Matrix4 mTransform = Engine::Matrix4::IDENTITY;
        std::chrono::microseconds mDuration = {};
        Engine::Color4 mColor = { 1, 1, 1, 1 };

        operator const MeshParameters &() const
        {
            return reinterpret_cast<const MeshParameters &>(*this);
        }
    };

    struct LineParameters {
        Engine::Matrix4 mTransform = Engine::Matrix4::IDENTITY;
        std::chrono::microseconds mDuration = {};
        Engine::Color4 mColorA = { 1, 1, 1, 1 };
        Engine::Color4 mColorB = mColorA;

        operator const MeshParameters &() const
        {
            return reinterpret_cast<const MeshParameters &>(*this);
        }
    };

    struct SphereParameters {
        Engine::Matrix4 mTransform = Engine::Matrix4::IDENTITY;
        std::chrono::microseconds mDuration = {};
        Engine::Color4 mColor = { 1, 1, 1, 1 };
        size_t mDetail = 2;

        operator const MeshParameters &() const
        {
            return reinterpret_cast<const MeshParameters &>(*this);
        }
    };

    struct TextParameters {
        Engine::Matrix4 mTransform = Engine::Matrix4::IDENTITY;
        std::chrono::microseconds mDuration = {};
        Engine::Color4 mColor = { 1, 1, 1, 1 };
        float mFontSize = 24;
        bool mFacingX = true;
        bool mFacingY = true;
        const char *mFontName = "OpenSans-Regular";
        Vector2 mPivot = { 0.5f, 0.5f };

        operator const MeshParameters &() const
        {
            return reinterpret_cast<const MeshParameters &>(*this);
        }
    };

}

}