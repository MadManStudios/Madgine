#include "../metalib.h"

#include "transformation.h"

#include "frustum.h"
#include "matrix4.h"
#include "quaternion.h"

namespace Engine {

Matrix4 TransformMatrix(const Vector3 &translate, const Vector3 &scale, const Quaternion &orientation)
{
    return TranslationMatrix(translate) * Matrix4 { orientation.toMatrix() * ScalingMatrix(scale) };
}

std::tuple<Vector3, Vector3, Quaternion> DecomposeTransformMatrix(const Matrix4 &transform)
{
    Vector3 translation = transform.GetColumn(3).xyz();
    Matrix3 m3 = transform.ToMat3();
    Vector3 scale = { m3.GetColumn(0).length(), m3.GetColumn(1).length(), m3.GetColumn(2).length() };
    Quaternion orientation = Quaternion::FromMatrix(ScalingMatrix(scale).Inverse() * m3);
    return { translation, scale, orientation };
}

Matrix4 TranslationMatrix(const Vector3 &v)
{
    return { 1, 0, 0, v.x, 0, 1, 0, v.y, 0, 0, 1, v.z, 0, 0, 0, 1 };
}

Matrix3 ScalingMatrix(const Vector3 &v)
{
    return { v.x, 0, 0, 0, v.y, 0, 0, 0, v.z };
}

Matrix4 ProjectionMatrix(const Frustum &frustum)
{
    float r = frustum.mRight;
    float t = frustum.mTop;
    float near = frustum.mNear;
    float far = frustum.mFar;

    Matrix4 p;

    if (frustum.mOrthographic) {
        p = {
            1 / r, 0, 0, 0,
            0, 1 / t, 0, 0,
            0, 0, 1 / (far - near), -1 * near / (far - near),
            0, 0, 0, 1
        };
    } else {
        p = {
            near / r, 0, 0, 0,
            0, near / t, 0, 0,
            0, 0, far / (far - near), -1 * far * near / (far - near),
            0, 0, 1, 0
        };
    }

    return p;
}

Matrix3 ExtractRotationMatrix(const Matrix3 &m)
{

    return m * ExtractScalingMatrix(m).Inverse();
}

Matrix3 ExtractScalingMatrix(const Matrix3 &m)
{
    Vector3 scalings = { m.GetColumn(0).length(), m.GetColumn(1).length(), m.GetColumn(2).length() };
    return ScalingMatrix(scalings);
}

}