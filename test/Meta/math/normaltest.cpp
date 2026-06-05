#include <gtest/gtest.h>

#include "Meta/metalib.h"

#include "Meta/math/matrix3.h"
#include "Meta/math/matrix4.h"

#include "Meta/math/quaternion.h"

#include "Meta/math/transformation.h"

TEST(Math, Normals)
{
    using namespace Engine;

    Math::Quaternion q = Math::Quaternion::FromDegrees({ 45, 90, 0 });
    Math::Quaternion q1 = Math::Quaternion::FromDegrees({ 0, 90, 0 });
    Math::Quaternion q2 = Math::Quaternion::FromDegrees({ 0, 0, 45 });

    Math::Matrix3 ref = q.toMatrix();
    Math::Matrix3 ref1 = q1.toMatrix();
    Math::Matrix3 ref2 = q2.toMatrix();

    EXPECT_EQ(q1 * q2, q);
    EXPECT_TRUE((ref1 * ref2).equalsWithEpsilon(ref));


    Math::Matrix4 m = TransformMatrix(Math::Vector3::ZERO, Math::Vector3::UNIT_SCALE, q);

    Math::Matrix4 inv_m = m.Inverse();
    Math::Matrix4 anti_m = inv_m.Transpose();

    EXPECT_TRUE((inv_m * m).equalsWithEpsilon(Math::Matrix4::IDENTITY));
    EXPECT_TRUE((m * inv_m).equalsWithEpsilon(Math::Matrix4::IDENTITY));

    Math::Vector4 n { 1, 0, 0, 0 };

    //Vector4 result = anti_m * n;


}