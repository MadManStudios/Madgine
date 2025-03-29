#pragma once

#include "Meta/math/quaternion.h"
#include "Meta/math/vector3.h"
#include "Meta/math/frustum.h"

namespace Engine {
namespace Render {

    struct MADGINE_RENDER_EXPORT Camera {
        Camera(std::string name = "");
        //Camera(const Camera &) = delete;
        ~Camera() = default;

        Matrix4 getViewProjectionMatrix(float aspectRatio);
        Matrix4 getViewMatrix();
        Matrix4 getProjectionMatrix(float aspectRatio);
        Frustum getFrustum(float aspectRatio) const;

        Ray3 mousePointToRay(const Vector2 &mousePos, const Vector2 &viewportSize);
        Ray3 toRay() const;

        std::string mName;

        Vector3 mPosition = Vector3::ZERO;
        Quaternion mOrientation;

        float mF = 200.0f;
        float mN = 0.1f;
        float mFOV = 90.0f;

        bool mOrthographic = false;
    };

}
}
