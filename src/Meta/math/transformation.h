#pragma once

namespace Engine {
namespace Math {

    META_EXPORT Matrix4 TransformMatrix(const Vector3 &translate, const Vector3 &scale, const Quaternion &orientation);
    META_EXPORT std::tuple<Vector3, Vector3, Quaternion> DecomposeTransformMatrix(const Matrix4 &transform);

    META_EXPORT Matrix4 TranslationMatrix(const Vector3 &v);
    META_EXPORT Matrix3 ScalingMatrix(const Vector3 &v);
    META_EXPORT Matrix4 ProjectionMatrix(const Frustum &frustum);

    META_EXPORT Matrix3 ExtractRotationMatrix(const Matrix3 &m);
    META_EXPORT Matrix3 ExtractScalingMatrix(const Matrix3 &m);

}
}