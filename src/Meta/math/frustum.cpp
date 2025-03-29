#include "../metalib.h"

#include "frustum.h"

#include "matrix4.h"

#include "transformation.h"

#include "ray3.h"

namespace Engine {

std::array<Vector3, 8> Frustum::getCorners() const
{

    float topFar = mTop;
    float rightFar = mRight;
    if (!mOrthographic) {
        topFar *= (mFar / mNear);
        rightFar *= (mFar / mNear);
    }

    std::array<Vector3, 8> result { { { -mRight, -mTop, mNear },
        { mRight, -mTop, mNear },
        { mRight, mTop, mNear },
        { -mRight, mTop, mNear },
        { -rightFar, -topFar, mFar },
        { rightFar, -topFar, mFar },
        { rightFar, topFar, mFar },
        { -rightFar, topFar, mFar } } };
    for (Vector3 &v : result) {
        v = mOrientation * v + mPosition;
    }

    return result;
}

Matrix4 Frustum::getViewMatrix() const
{
    return Matrix4 { mOrientation.inverse().toMatrix() } * TranslationMatrix(-mPosition);
}

Matrix4 Frustum::getProjectionMatrix() const
{
    return ProjectionMatrix(*this);
}

Matrix4 Frustum::getViewProjectionMatrix() const
{
    return getProjectionMatrix() * getViewMatrix();
}

Ray3 Frustum::toRay(const Vector2& uv) const {

    Vector3 dir = mOrientation * Vector3 { (2.f * uv.x - 1.0f) * mRight, (1.0f - 2.f * uv.y) * mTop, mNear };

    Vector3 pos = mPosition + dir;

    if (mOrthographic) {
        return { pos, mOrientation * Vector3 { Vector3::UNIT_Z } };
    } else {
        return { pos, dir };
    }
}

}