#pragma once

#include "Meta/math/frustum.h"
#include "Meta/math/quaternion.h"
#include "Meta/math/vector3.h"

namespace Engine {
namespace Render {

    struct MADGINE_RENDER_EXPORT Camera {
        Camera(std::string name = "");
        // Camera(const Camera &) = delete;
        ~Camera() = default;

        Math::Matrix4 getViewProjectionMatrix(float aspectRatio) const;
        Math::Matrix4 getViewMatrix() const;
        Math::Matrix4 getProjectionMatrix(float aspectRatio) const;
        Math::Frustum getFrustum(float aspectRatio) const;

        Math::Vector2 worldToScreen(const Math::Vector3 &worldPos, const Math::Vector2 &viewportSize) const;
        Math::Ray3 mousePointToRay(const Math::Vector2 &mousePos, const Math::Vector2 &viewportSize) const;
        Math::Ray3 toRay() const;

        std::string mName;

        Math::Vector3 mPosition = Math::Vector3::ZERO;
        Math::Quaternion mOrientation;

        float mF = 200.0f;
        float mN = 0.1f;
        float mFOV = 60.0f;

        bool mOrthographic = false;
    };

}
}
