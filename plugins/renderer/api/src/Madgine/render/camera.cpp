#include "../renderlib.h"

#include "camera.h"

#include "Meta/math/matrix4.h"
#include "Meta/math/pi.h"
#include "Meta/math/ray3.h"
#include "Meta/math/transformation.h"

#include "Meta/reflect/metatable_impl.h"

METATABLE_BEGIN(Engine::Render::Camera)
    //CONSTRUCTOR()
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

    Math::Matrix4 Camera::getViewProjectionMatrix(float aspectRatio)
    {
        return getProjectionMatrix(aspectRatio) * getViewMatrix();
    }

    Math::Matrix4 Camera::getViewMatrix()
    {
        return Math::Matrix4 { mOrientation.inverse().toMatrix() } * TranslationMatrix(-mPosition);
    }

    Math::Matrix4 Camera::getProjectionMatrix(float aspectRatio)
    {
        return getFrustum(aspectRatio).getProjectionMatrix();
    }

    Math::Frustum Camera::getFrustum(float aspectRatio) const
    {
        float r = tanf((mFOV / 180.0f * Math::PI) / 2.0f) * mN;
        float t = r / aspectRatio;

        return { mPosition,
            mOrientation,
            t, r,
            mN, mF,
            mOrthographic };
    }

    Math::Ray3 Camera::mousePointToRay(const Math::Vector2 &mousePos, const Math::Vector2 &viewportSize)
    {
        float aspectRatio = viewportSize.x / viewportSize.y;

        return getFrustum(aspectRatio).toRay(mousePos / viewportSize);
    }

    Math::Ray3 Camera::toRay() const
    {
        Math::Vector3 dir = mOrientation * Math::Vector3 { 0, 0, mN };
        return { mPosition, dir };
    }

}
}
