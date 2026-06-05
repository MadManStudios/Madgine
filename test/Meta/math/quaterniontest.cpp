#include <gtest/gtest.h>

#include "Meta/metalib.h"

#include "Meta/math/quaternion.h"

TEST(Math, Quaternion)
{
    using namespace Engine;

    Math::Quaternion q1 { Math::PI / 2, Math::Vector3::UNIT_Y };

    Math::Matrix3 rot1 {
        0, 0, 1,
        0, 1, 0,
        -1, 0, 0
    };

    ASSERT_TRUE(q1.toMatrix().equalsWithEpsilon(rot1));

    Math::NormalizedVector3 dir { 1, 1, 1 };

    Math::Quaternion dirQ = Math::Quaternion::FromDirection(dir);

     

    ASSERT_TRUE((dirQ.toMatrix() * Math::Vector3 { Math::Vector3::UNIT_Z }).equalsWithEpsilon(dir));
}
