//-----------------------------------------------------------------------------
// File : TransformTest.cpp
// Desc : Transform Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxTrasnform.h>


TEST(TransformTest, DefaultConstructed)
{
    const asdx::Transform transform;

    EXPECT_FLOAT_EQ(transform.Position.x, 0.0f);
    EXPECT_FLOAT_EQ(transform.Position.y, 0.0f);
    EXPECT_FLOAT_EQ(transform.Position.z, 0.0f);
    EXPECT_FLOAT_EQ(transform.Scale.x, 1.0f);
    EXPECT_FLOAT_EQ(transform.Scale.y, 1.0f);
    EXPECT_FLOAT_EQ(transform.Scale.z, 1.0f);
    EXPECT_FLOAT_EQ(transform.Rotation.x, 0.0f);
    EXPECT_FLOAT_EQ(transform.Rotation.y, 0.0f);
    EXPECT_FLOAT_EQ(transform.Rotation.z, 0.0f);
    EXPECT_FLOAT_EQ(transform.Rotation.w, 1.0f);
}

TEST(TransformTest, CalcMatrix4x4)
{
    asdx::Transform transform;
    transform.Position = asdx::Vector3(10.0f, -20.0f, 30.0f);
    transform.Scale = asdx::Vector3(2.0f, 3.0f, 4.0f);

    const auto matrix = asdx::CalcMatrix4x4(transform);

    EXPECT_FLOAT_EQ(matrix._11, 2.0f);
    EXPECT_FLOAT_EQ(matrix._12, 0.0f);
    EXPECT_FLOAT_EQ(matrix._13, 0.0f);
    EXPECT_FLOAT_EQ(matrix._14, 0.0f);
    EXPECT_FLOAT_EQ(matrix._21, 0.0f);
    EXPECT_FLOAT_EQ(matrix._22, 3.0f);
    EXPECT_FLOAT_EQ(matrix._23, 0.0f);
    EXPECT_FLOAT_EQ(matrix._24, 0.0f);
    EXPECT_FLOAT_EQ(matrix._31, 0.0f);
    EXPECT_FLOAT_EQ(matrix._32, 0.0f);
    EXPECT_FLOAT_EQ(matrix._33, 4.0f);
    EXPECT_FLOAT_EQ(matrix._34, 0.0f);
    EXPECT_FLOAT_EQ(matrix._41, 10.0f);
    EXPECT_FLOAT_EQ(matrix._42, -20.0f);
    EXPECT_FLOAT_EQ(matrix._43, 30.0f);
    EXPECT_FLOAT_EQ(matrix._44, 1.0f);
}

TEST(TransformTest, CalcMatrix4x3)
{
    asdx::Transform transform;
    transform.Position = asdx::Vector3(10.0f, -20.0f, 30.0f);
    transform.Scale = asdx::Vector3(2.0f, 3.0f, 4.0f);

    const auto matrix = asdx::CalcMatrix4x3(transform);

    EXPECT_FLOAT_EQ(matrix._11, 2.0f);
    EXPECT_FLOAT_EQ(matrix._12, 0.0f);
    EXPECT_FLOAT_EQ(matrix._13, 0.0f);
    EXPECT_FLOAT_EQ(matrix._21, 0.0f);
    EXPECT_FLOAT_EQ(matrix._22, 3.0f);
    EXPECT_FLOAT_EQ(matrix._23, 0.0f);
    EXPECT_FLOAT_EQ(matrix._31, 0.0f);
    EXPECT_FLOAT_EQ(matrix._32, 0.0f);
    EXPECT_FLOAT_EQ(matrix._33, 4.0f);
    EXPECT_FLOAT_EQ(matrix._41, 10.0f);
    EXPECT_FLOAT_EQ(matrix._42, -20.0f);
    EXPECT_FLOAT_EQ(matrix._43, 30.0f);
}

TEST(TransformTest, CalcMatrixWithRotation)
{
    asdx::Transform transform;
    transform.Scale = asdx::Vector3(2.0f, 3.0f, 4.0f);
    transform.Rotation = asdx::Quaternion::CreateFromAxisAngle(
        asdx::Vector3(0.0f, 1.0f, 0.0f), asdx::F_PIDIV2);

    const auto matrix = asdx::CalcMatrix4x4(transform);

    EXPECT_NEAR(matrix._11, 0.0f, 1.0e-6f);
    EXPECT_FLOAT_EQ(matrix._12, 0.0f);
    EXPECT_NEAR(matrix._13, -2.0f, 1.0e-6f);
    EXPECT_FLOAT_EQ(matrix._21, 0.0f);
    EXPECT_FLOAT_EQ(matrix._22, 3.0f);
    EXPECT_FLOAT_EQ(matrix._23, 0.0f);
    EXPECT_NEAR(matrix._31, 4.0f, 1.0e-6f);
    EXPECT_FLOAT_EQ(matrix._32, 0.0f);
    EXPECT_NEAR(matrix._33, 0.0f, 1.0e-6f);
}

