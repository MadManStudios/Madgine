#include <gtest/gtest.h>

#include "Meta/metalib.h"

#include "Meta/math/matrix3.h"
#include "Meta/math/matrix4.h"

TEST(Math, Matrix3)
{
    using namespace Engine;

    Math::Matrix3 identity {
        1, 0, 0,
        0, 1, 0,
        0, 0, 1
    };

    Math::Matrix3 rot1 {
        0, 1, 0,
        0, 0, 1,
        1, 0, 0
    };

    Math::Matrix3 rot2 {
        0, 0, 1,
        1, 0, 0,
        0, 1, 0
    };

    Math::Matrix3 flip1 {
        0, 1, 0,
        1, 0, 0,
        0, 0, 1
    };

    Math::Matrix3 flip2 {
        1, 0, 0,
        0, 0, 1,
        0, 1, 0
    };

    Math::Matrix3 flip3 {
        0, 0, 1,
        0, 1, 0,
        1, 0, 0
    };

    Math::Matrix3 ones {
        1, 1, 1,
        1, 1, 1,
        1, 1, 1
    };

    EXPECT_EQ(identity * identity, identity);
    EXPECT_EQ(identity * rot1, rot1);
    EXPECT_EQ(rot1 * identity, rot1);
    EXPECT_EQ(rot1 * rot1, rot2);
    EXPECT_EQ(rot1 * rot2, identity);

    EXPECT_EQ(ones * ones, 3 * ones);

    EXPECT_EQ(flip1.Inverse(), flip1);
    EXPECT_EQ(flip2.Inverse(), flip2);
    EXPECT_EQ(flip3.Inverse(), flip3);

    EXPECT_EQ(rot1.Inverse(), rot2);
    EXPECT_EQ(rot2.Inverse(), rot1);

    EXPECT_EQ(ones.Determinant(), 0);

    EXPECT_EQ(rot1[0][1], 1);
    EXPECT_EQ(rot1[1][0], 0);
}

TEST(Math, Matrix4)
{
    using namespace Engine;

    Math::Matrix4 identity {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    Math::Matrix4 rot1 {
        0, 1, 0, 0,
        0, 0, 1, 0,
        1, 0, 0, 0,
        0, 0, 0, 1
    };

    Math::Matrix4 rot2 {
        0, 0, 1, 0,
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 0, 1
    };

    Math::Matrix4 flip1 {
        0, 1, 0, 0,
        1, 0, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    Math::Matrix4 flip2 {
        1, 0, 0, 0,
        0, 0, 1, 0,
        0, 1, 0, 0,
        0, 0, 0, 1
    };

    Math::Matrix4 flip3 {
        0, 0, 1, 0,
        0, 1, 0, 0,
        1, 0, 0, 0,
        0, 0, 0, 1
    };

    Math::Matrix4 ones {
        1, 1, 1, 1,
        1, 1, 1, 1,
        1, 1, 1, 1,
        1, 1, 1, 1
    };

    EXPECT_EQ(identity * identity, identity);
    EXPECT_EQ(identity * rot1, rot1);
    EXPECT_EQ(rot1 * identity, rot1);
    EXPECT_EQ(rot1 * rot1, rot2);
    EXPECT_EQ(rot1 * rot2, identity);

    EXPECT_EQ(ones * ones, 4 * ones);

    EXPECT_EQ(flip1.Inverse(), flip1);
    EXPECT_EQ(flip2.Inverse(), flip2);
    EXPECT_EQ(flip3.Inverse(), flip3);

    EXPECT_EQ(rot1.Inverse(), rot2);
    EXPECT_EQ(rot2.Inverse(), rot1);

    EXPECT_EQ(rot1[0][1], 1);
    EXPECT_EQ(rot1[1][0], 0);

    Math::Matrix3 tmp = { 1, 2, 3,
                    4, 5, 6,
                    7, 8, 9 };
    EXPECT_EQ(tmp, Math::Matrix4 { tmp }.ToMat3());

    Math::Matrix4 transposeX {
        1, 0, 0, 1,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    Math::Matrix4 invTransposeX {
        1, 0, 0, -1,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    EXPECT_EQ(transposeX.Inverse(), invTransposeX);

    Math::Matrix4 transposeY {
        1, 0, 0, 0,
        0, 1, 0, 1,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    Math::Matrix4 invTransposeY {
        1, 0, 0, 0,
        0, 1, 0, -1,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    EXPECT_EQ(transposeY.Inverse(), invTransposeY);

    Math::Matrix4 transposeZ {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 1,
        0, 0, 0, 1
    };

    Math::Matrix4 invTransposeZ {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, -1,
        0, 0, 0, 1
    };

    EXPECT_EQ(transposeZ.Inverse(), invTransposeZ);

    EXPECT_EQ((rot1 * transposeX).Inverse(), invTransposeX * rot2);
    EXPECT_EQ((rot1 * transposeY).Inverse(), invTransposeY * rot2);
    EXPECT_EQ((rot1 * transposeZ).Inverse(), invTransposeZ * rot2);
}
