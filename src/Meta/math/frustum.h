#pragma once

#include "quaternion.h"
#include "vector3.h"

namespace Engine {

struct META_EXPORT Frustum {

    Vector3 mPosition;
    Quaternion mOrientation;
    float mTop, mRight;
    float mNear, mFar;
    bool mOrthographic;

    std::array<Vector3, 8> getCorners() const;
    Matrix4 getViewMatrix() const;
    Matrix4 getProjectionMatrix() const;
    Matrix4 getViewProjectionMatrix() const;
    Ray3 toRay(const Vector2 &uv) const;
};

inline Frustum operator*(const Quaternion &q, const Frustum &f)
{
    return {
        q * f.mPosition,
        q * f.mOrientation,
        f.mTop, f.mRight,
        f.mNear, f.mFar,
        f.mOrthographic
    };
}

}