#pragma once

#include "matrix4.h"
#include "vector3.h"

namespace Engine {
namespace Math {

    struct Sphere {

        Vector3 mCenter;
        float mRadius;
    };

    inline Sphere operator*(const Matrix4 &m, const Sphere &s)
    {
        return {
            (m * Vector4 { s.mCenter, 1.0f }).xyz(),
            s.mRadius
        }; // TODO scaling
    }

}
}