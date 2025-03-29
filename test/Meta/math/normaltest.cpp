#include <gtest/gtest.h>

#include "Meta/metalib.h"

#include "Meta/math/matrix3.h"
#include "Meta/math/matrix4.h"

#include "Meta/math/quaternion.h"

#include "Meta/math/transformation.h"

TEST(Math, Normals)
{
    using namespace Engine;

    Quaternion q = Quaternion::FromDegrees({ 45, 90, 0 });
    Quaternion q1 = Quaternion::FromDegrees({ 0, 90, 0 });
    Quaternion q2 = Quaternion::FromDegrees({ 0, 0, 45 });

    Matrix3 ref = q.toMatrix();
    Matrix3 ref1 = q1.toMatrix();
    Matrix3 ref2 = q2.toMatrix();

    EXPECT_EQ(q1 * q2, q);
    EXPECT_TRUE((ref1 * ref2).equalsWithEpsilon(ref));


    Matrix4 m = TransformMatrix(Vector3::ZERO, Vector3::UNIT_SCALE, q);

    Matrix4 inv_m = m.Inverse();
    Matrix4 anti_m = inv_m.Transpose();

    EXPECT_TRUE((inv_m * m).equalsWithEpsilon(Matrix4::IDENTITY));
    EXPECT_TRUE((m * inv_m).equalsWithEpsilon(Matrix4::IDENTITY));

    Vector4 n { 1, 0, 0, 0 };

    Vector4 result = anti_m * n;


}