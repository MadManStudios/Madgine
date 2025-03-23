#include "../renderlib.h"

#include "camera.h"

#include "Meta/math/matrix4.h"

#include "Meta/keyvalue/metatable_impl.h"

#include "Meta/math/pi.h"

#include "Meta/math/ray3.h"

#include "Meta/math/transformation.h"

METATABLE_BEGIN(Engine::Render::Camera)
CONSTRUCTOR()
MEMBER(mName)
MEMBER(mPosition)
MEMBER(mOrientation)
MEMBER(mN)
MEMBER(mF)
MEMBER(mFOV)
MEMBER(mOrthographic)
FUNCTION(getViewMatrix)
FUNCTION(getViewProjectionMatrix, aspectRatio)
METATABLE_END(Engine::Render::Camera)

namespace Engine {
namespace Render {

    Camera::Camera(std::string name)
        : mName(std::move(name))
    {
    }

    Matrix4 Camera::getViewProjectionMatrix(float aspectRatio)
    {
        return getProjectionMatrix(aspectRatio) * getViewMatrix();
    }

    Matrix4 Camera::getViewMatrix()
    {
        return Matrix4 { mOrientation.inverse().toMatrix() } * TranslationMatrix(-mPosition);
    }

    Matrix4 Camera::getProjectionMatrix(float aspectRatio)
    {
        return getFrustum(aspectRatio).getProjectionMatrix();
    }

    Frustum Camera::getFrustum(float aspectRatio) const
    {
        float r = tanf((mFOV / 180.0f * PI) / 2.0f) * mN;
        float t = r / aspectRatio;

        return { mPosition,
            mOrientation,
            t, r,
            mN, mF,
            mOrthographic };
    }

    Ray3 Camera::mousePointToRay(const Vector2 &mousePos, const Vector2 &viewportSize)
    {
        float aspectRatio = viewportSize.x / viewportSize.y;

        return getFrustum(aspectRatio).toRay(mousePos / viewportSize);
    }

    Ray3 Camera::toRay() const
    {
        Vector3 dir = mOrientation * Vector3 { 0, 0, mN };
        return { mPosition, dir };
    }

}
}
