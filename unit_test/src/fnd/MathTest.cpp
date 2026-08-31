//-----------------------------------------------------------------------------
// File : MathTest.cpp
// Desc : Math Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxMath.h>

TEST(MathTest, ScalarFunctions)
{
    EXPECT_NEAR(asdx::ToRadian(180.0f), asdx::F_PI, 1.0e-6f);
    EXPECT_NEAR(asdx::ToDegree(asdx::F_PIDIV2), 90.0f, 1.0e-5f);
    EXPECT_NEAR(asdx::ToRadian(180.0), asdx::D_PI, 1.0e-12);
    EXPECT_NEAR(asdx::ToDegree(asdx::D_PIDIV2), 90.0, 1.0e-12);

    EXPECT_TRUE(asdx::IsZero(0.0f));
    EXPECT_TRUE(asdx::IsZero(asdx::F_EPSILON));
    EXPECT_FALSE(asdx::IsZero(1.0e-3f));
    EXPECT_TRUE(asdx::IsEqual(1.0f, 1.0f + asdx::F_EPSILON));
    EXPECT_FALSE(asdx::IsEqual(1.0f, 1.0f + 1.0e-3f));
    EXPECT_TRUE(asdx::IsNaN(NAN));
    EXPECT_FALSE(asdx::IsNaN(1.0f));
    EXPECT_TRUE(asdx::IsInf(INFINITY));
    EXPECT_FALSE(asdx::IsInf(1.0f));

    EXPECT_NEAR(asdx::Hypot(3.0f, 4.0f), 5.0f, 1.0e-6f);
    EXPECT_NEAR(asdx::Hypot(1.0f, 2.0f, 2.0f), 3.0f, 1.0e-6f);
    EXPECT_NEAR(asdx::Hypot(1.0, 2.0, 2.0, 2.0), 3.60555127546, 1.0e-10);

    EXPECT_EQ(asdx::ToHalf(0.0f), 0x0000u);
    EXPECT_EQ(asdx::ToHalf(1.0f), 0x3c00u);
    EXPECT_EQ(asdx::ToHalf(-2.0f), 0xc000u);
    EXPECT_FLOAT_EQ(asdx::ToFloat(asdx::ToHalf(1.5f)), 1.5f);
}

TEST(MathTest, ScalarUtilities)
{
    EXPECT_EQ(asdx::Fact(0), 1u);
    EXPECT_EQ(asdx::Fact(5), 120u);
    EXPECT_EQ(asdx::DblFact(6), 48u);
    EXPECT_EQ(asdx::Perm(5, 2), 20u);
    EXPECT_EQ(asdx::Comb(5, 2), 10u);

    EXPECT_FLOAT_EQ(asdx::Lerp(2.0f, 10.0f, 0.25f), 4.0f);
    EXPECT_EQ(asdx::Clamp(12, 0, 10), 10);
    EXPECT_EQ(asdx::Clamp(-1, 0, 10), 0);
    EXPECT_EQ(asdx::Wrap(12, 0, 10), 2);
    EXPECT_EQ(asdx::Wrap(-1, 0, 10), 9);
    EXPECT_EQ(asdx::Saturate(-1.0f), 0.0f);
    EXPECT_EQ(asdx::Saturate(2.0f), 1.0f);
    EXPECT_EQ(asdx::Sign(-3), -1);
    EXPECT_EQ(asdx::RoundUp(13, 8), 16);
    EXPECT_EQ(asdx::RoundDown(13, 8), 8);
    EXPECT_EQ(asdx::RoundDiv(13, 8), 2);
    EXPECT_TRUE(asdx::IsPowerOf2(16u));
    EXPECT_FALSE(asdx::IsPowerOf2(0u));
}

TEST(MathTest, Vector2Operations)
{
    const asdx::Vector2 a(3.0f, 4.0f);
    const asdx::Vector2 b(-1.0f, 2.0f);
    const auto sum = a + b;
    const auto product = a * b;

    EXPECT_FLOAT_EQ(sum.x, 2.0f);
    EXPECT_FLOAT_EQ(sum.y, 6.0f);
    EXPECT_FLOAT_EQ(product.x, -3.0f);
    EXPECT_FLOAT_EQ(product.y, 8.0f);
    EXPECT_FLOAT_EQ(a.LengthSq(), 25.0f);
    EXPECT_FLOAT_EQ(a.Length(), 5.0f);
    EXPECT_FLOAT_EQ(asdx::Vector2::Dot(a, b), 5.0f);
    EXPECT_NEAR(asdx::Vector2::Distance(a, b), 4.47213595f, 1.0e-6f);

    const auto normalized = asdx::Vector2::Normalize(a);
    EXPECT_FLOAT_EQ(normalized.x, 0.6f);
    EXPECT_FLOAT_EQ(normalized.y, 0.8f);
    const auto zero = asdx::Vector2::SafeNormalize(asdx::Vector2(), asdx::Vector2(1.0f, 2.0f));
    EXPECT_FLOAT_EQ(zero.x, 1.0f);
    EXPECT_FLOAT_EQ(zero.y, 2.0f);
}

TEST(MathTest, Vector3Operations)
{
    const asdx::Vector3 x(1.0f, 0.0f, 0.0f);
    const asdx::Vector3 y(0.0f, 1.0f, 0.0f);
    const auto cross = asdx::Vector3::Cross(x, y);

    EXPECT_FLOAT_EQ(cross.x, 0.0f);
    EXPECT_FLOAT_EQ(cross.y, 0.0f);
    EXPECT_FLOAT_EQ(cross.z, 1.0f);
    EXPECT_FLOAT_EQ(asdx::Vector3::Dot(x, y), 0.0f);
    EXPECT_NEAR(asdx::Vector3::ComputeCrossingAngle(x, y), asdx::F_PIDIV2, 1.0e-6f);

    const auto clamped = asdx::Vector3::Clamp(
        asdx::Vector3(-1.0f, 0.5f, 2.0f), asdx::Vector3(0.0f), asdx::Vector3(1.0f));
    EXPECT_FLOAT_EQ(clamped.x, 0.0f);
    EXPECT_FLOAT_EQ(clamped.y, 0.5f);
    EXPECT_FLOAT_EQ(clamped.z, 1.0f);
    const auto normal = asdx::Vector3::ComputeNormal(
        asdx::Vector3(0.0f, 0.0f, 0.0f),
        asdx::Vector3(1.0f, 0.0f, 0.0f),
        asdx::Vector3(0.0f, 1.0f, 0.0f));
    EXPECT_FLOAT_EQ(normal.x, 0.0f);
    EXPECT_FLOAT_EQ(normal.y, 0.0f);
    EXPECT_FLOAT_EQ(normal.z, 1.0f);
}

TEST(MathTest, VectorInterpolation)
{
    const asdx::Vector4 a(0.0f, 2.0f, 4.0f, 6.0f);
    const asdx::Vector4 b(10.0f, 12.0f, 14.0f, 16.0f);
    const auto value = asdx::Vector4::Lerp(a, b, 0.25f);

    EXPECT_FLOAT_EQ(value.x, 2.5f);
    EXPECT_FLOAT_EQ(value.y, 4.5f);
    EXPECT_FLOAT_EQ(value.z, 6.5f);
    EXPECT_FLOAT_EQ(value.w, 8.5f);
    const auto rgba = asdx::Vector4::FromRGBA(255, 128, 0, 64);
    EXPECT_FLOAT_EQ(rgba.x, 1.0f);
    EXPECT_NEAR(rgba.y, 128.0f / 255.0f, 1.0e-6f);
    EXPECT_FLOAT_EQ(rgba.z, 0.0f);
    EXPECT_NEAR(rgba.w, 64.0f / 255.0f, 1.0e-6f);
}

TEST(MathTest, MatrixOperations)
{
    const auto identity = asdx::Matrix4x4::CreateIdentity();
    EXPECT_FLOAT_EQ(identity._11, 1.0f);
    EXPECT_FLOAT_EQ(identity._22, 1.0f);
    EXPECT_FLOAT_EQ(identity._33, 1.0f);
    EXPECT_FLOAT_EQ(identity._44, 1.0f);
    EXPECT_FLOAT_EQ(identity.Determinant(), 1.0f);

    const auto scale = asdx::Matrix4x4::CreateScale(2.0f, 3.0f, 4.0f);
    const auto translation = asdx::Matrix4x4::CreateTranslation(10.0f, -20.0f, 30.0f);
    const auto transformed = asdx::Vector3::Transform(asdx::Vector3(1.0f, 2.0f, 3.0f), scale * translation);
    EXPECT_FLOAT_EQ(transformed.x, 12.0f);
    EXPECT_FLOAT_EQ(transformed.y, -14.0f);
    EXPECT_FLOAT_EQ(transformed.z, 42.0f);

    const auto rotation = asdx::Matrix4x4::CreateRotationZ(asdx::F_PIDIV2);
    const auto rotated = asdx::Vector3::Transform(asdx::Vector3(1.0f, 0.0f, 0.0f), rotation);
    EXPECT_NEAR(rotated.x, 0.0f, 1.0e-6f);
    EXPECT_NEAR(rotated.y, 1.0f, 1.0e-6f);
    EXPECT_FLOAT_EQ(rotated.z, 0.0f);
}

TEST(MathTest, QuaternionOperations)
{
    const auto identity = asdx::Quaternion::CreateIdentity();
    EXPECT_TRUE(asdx::Quaternion::IsIdentity(identity));
    EXPECT_TRUE(asdx::Quaternion::IsNormalized(identity));

    const auto rotation = asdx::Quaternion::CreateFromAxisAngle(
        asdx::Vector3(0.0f, 0.0f, 1.0f), asdx::F_PIDIV2);
    const auto matrix = asdx::Matrix4x4::CreateFromQuaternion(rotation);
    const auto rotated = asdx::Vector3::Transform(asdx::Vector3(1.0f, 0.0f, 0.0f), matrix);
    EXPECT_NEAR(rotated.x, 0.0f, 1.0e-6f);
    EXPECT_NEAR(rotated.y, 1.0f, 1.0e-6f);
    EXPECT_NEAR(rotated.z, 0.0f, 1.0e-6f);

    const auto conjugate = asdx::Quaternion::Conjugate(rotation);
    const auto product = asdx::Quaternion::Multiply(rotation, conjugate);
    EXPECT_NEAR(product.x, 0.0f, 1.0e-6f);
    EXPECT_NEAR(product.y, 0.0f, 1.0e-6f);
    EXPECT_NEAR(product.z, 0.0f, 1.0e-6f);
    EXPECT_NEAR(product.w, 1.0f, 1.0e-6f);
    EXPECT_NEAR(asdx::Quaternion::Dot(rotation, rotation), 1.0f, 1.0e-6f);
}

TEST(MathTest, GeometryFunctions)
{
    const auto plane = asdx::Vector4::NormalizePlane(asdx::Vector4(0.0f, 2.0f, 0.0f, 4.0f));
    EXPECT_FLOAT_EQ(plane.x, 0.0f);
    EXPECT_FLOAT_EQ(plane.y, 1.0f);
    EXPECT_FLOAT_EQ(plane.z, 0.0f);
    EXPECT_FLOAT_EQ(plane.w, 2.0f);

    const auto sample = asdx::Hammersley(1, 4);
    EXPECT_FLOAT_EQ(sample.x, 0.25f);
    EXPECT_FLOAT_EQ(sample.y, 0.5f);
}

TEST(MathTest, BoundingVolumes)
{
    const asdx::BoundingBox2 box2(asdx::Vector2(-1.0f, -2.0f), asdx::Vector2(3.0f, 4.0f));
    EXPECT_FLOAT_EQ(box2.GetCenter().x, 1.0f);
    EXPECT_FLOAT_EQ(box2.GetCenter().y, 1.0f);
    EXPECT_FLOAT_EQ(box2.GetSize().x, 4.0f);
    EXPECT_FLOAT_EQ(box2.GetSize().y, 6.0f);
    EXPECT_TRUE(box2.Contains(asdx::Vector2(0.0f, 0.0f)));
    EXPECT_FALSE(box2.Contains(asdx::Vector2(4.0f, 0.0f)));
    EXPECT_EQ(box2.GetCorners().size(), 4u);
    const auto merged2 = asdx::BoundingBox2::Merge(box2, asdx::Vector2(5.0f, -3.0f));
    EXPECT_FLOAT_EQ(merged2.Maxi.x, 5.0f);
    EXPECT_FLOAT_EQ(merged2.Mini.y, -3.0f);

    const asdx::BoundingBox3 box3(asdx::Vector3(-1.0f), asdx::Vector3(1.0f));
    EXPECT_TRUE(box3.Contains(asdx::Vector3(0.0f)));
    EXPECT_FALSE(box3.Contains(asdx::Vector3(2.0f, 0.0f, 0.0f)));
    EXPECT_FLOAT_EQ(box3.GetSize().x, 2.0f);
    EXPECT_EQ(box3.GetCorners().size(), 8u);

    const asdx::BoundingSphere2 sphere2(asdx::Vector2(0.0f), 2.0f);
    EXPECT_TRUE(sphere2.Contains(asdx::Vector2(1.0f, 1.0f)));
    EXPECT_FALSE(sphere2.Contains(asdx::Vector2(2.0f, 2.0f)));
    const auto mergedSphere = asdx::BoundingSphere2::Merge(sphere2, asdx::Vector2(4.0f, 0.0f));
    EXPECT_NEAR(mergedSphere.Center.x, 1.0f, 1.0e-6f);
    EXPECT_NEAR(mergedSphere.Radius, 3.0f, 1.0e-6f);

    const asdx::BoundingSphere3 sphere3(asdx::Vector3(0.0f), 2.0f);
    EXPECT_TRUE(sphere3.Contains(asdx::Vector3(1.0f, 0.0f, 0.0f)));
    EXPECT_FALSE(sphere3.Contains(asdx::Vector3(2.0f, 2.0f, 0.0f)));
    const auto moved = asdx::BoundingSphere3::Transform(
        sphere3, asdx::Matrix4x4::CreateTranslation(3.0f, 4.0f, 5.0f));
    EXPECT_FLOAT_EQ(moved.Center.x, 3.0f);
    EXPECT_FLOAT_EQ(moved.Center.y, 4.0f);
    EXPECT_FLOAT_EQ(moved.Center.z, 5.0f);
    EXPECT_FLOAT_EQ(moved.Radius, sphere3.Radius);
    const float vertices[] = { -2.0f, -1.0f, -3.0f, 4.0f, 5.0f, 6.0f };
    const auto createdBox = asdx::BoundingBox3::Create(vertices, 2, 3 * sizeof(float));
    EXPECT_FLOAT_EQ(createdBox.Mini.x, -2.0f);
    EXPECT_FLOAT_EQ(createdBox.Mini.y, -1.0f);
    EXPECT_FLOAT_EQ(createdBox.Mini.z, -3.0f);
    EXPECT_FLOAT_EQ(createdBox.Maxi.x, 4.0f);
    EXPECT_FLOAT_EQ(createdBox.Maxi.y, 5.0f);
    EXPECT_FLOAT_EQ(createdBox.Maxi.z, 6.0f);
    const auto createdSphere = asdx::BoundingSphere3::Create(vertices, 2, 3 * sizeof(float));
    EXPECT_TRUE(createdSphere.Contains(asdx::Vector3(-2.0f, -1.0f, -3.0f)));
    EXPECT_TRUE(createdSphere.Contains(asdx::Vector3(4.0f, 5.0f, 6.0f)));
}

TEST(MathTest, RandomGenerators)
{
    asdx::XorShift xorshift(1234u);
    const auto first = xorshift.GetValue();
    asdx::XorShift copy(xorshift);
    EXPECT_EQ(copy.GetValue(), xorshift.GetValue());
    xorshift.SetSeed(1234u);
    EXPECT_EQ(xorshift.GetValue(), first);
    xorshift.SetState(1u, 2u, 3u, 4u);
    uint32_t x = 0, y = 0, z = 0, w = 0;
    xorshift.GetState(x, y, z, w);
    EXPECT_EQ(x, 1u);
    EXPECT_EQ(y, 2u);
    EXPECT_EQ(z, 3u);
    EXPECT_EQ(w, 4u);

    asdx::PCG pcg(1234u);
    const auto pcgFirst = pcg.GetValue();
    asdx::PCG pcgCopy(pcg);
    EXPECT_EQ(pcgCopy.GetValue(), pcg.GetValue());
    pcg.SetSeed(1234u);
    EXPECT_EQ(pcg.GetValue(), pcgFirst);
    pcg.SetState(42u);
    EXPECT_EQ(pcg.GetState(), 42u);
}

TEST(MathTest, QuadAndUnorm)
{
    asdx::Quad2 quad(10, 20, 30, 40);
    EXPECT_TRUE(asdx::Quad2::Contains(10, 20, quad));
    EXPECT_FALSE(asdx::Quad2::Contains(41, 61, quad));
    const auto nested = asdx::Quad2(15, 25, 5, 5);
    EXPECT_TRUE(asdx::Quad2::Contains(quad, nested));
    quad.Move(-5, 10);
    EXPECT_EQ(quad.x, 5);
    EXPECT_EQ(quad.y, 30);

    const asdx::Unorm2 unorm2(asdx::Vector2(0.5f, 1.0f));
    EXPECT_EQ(unorm2.x, 127);
    EXPECT_EQ(unorm2.y, 255);
    EXPECT_NEAR(unorm2.ToVector2().x, 127.0f / 255.0f, 1.0e-6f);
    const auto rgb = asdx::Unorm3::FromRGB(0x00123456u);
    EXPECT_EQ(rgb.x, 0x12);
    EXPECT_EQ(rgb.y, 0x34);
    EXPECT_EQ(rgb.z, 0x56);
    const auto rgba = asdx::Unorm4::FromRGBA(0x12345678u);
    EXPECT_EQ(rgba.x, 0x12);
    EXPECT_EQ(rgba.y, 0x34);
    EXPECT_EQ(rgba.z, 0x56);
    EXPECT_EQ(rgba.w, 0x78);
}

TEST(MathTest, OrthonormalBasisAndIntersection)
{
    const asdx::Vector3 normal(0.0f, 1.0f, 0.0f);
    asdx::Vector3 tangent;
    asdx::Vector3 bitangent;
    asdx::CalcONB(normal, tangent, bitangent);
    EXPECT_NEAR(asdx::Vector3::Dot(normal, tangent), 0.0f, 1.0e-6f);
    EXPECT_NEAR(asdx::Vector3::Dot(normal, bitangent), 0.0f, 1.0e-6f);
    EXPECT_NEAR(asdx::Vector3::Dot(tangent, bitangent), 0.0f, 1.0e-6f);
    EXPECT_NEAR(tangent.Length(), 1.0f, 1.0e-6f);
    EXPECT_NEAR(bitangent.Length(), 1.0f, 1.0e-6f);

    const auto intersection = asdx::ComputeIntersection(
        asdx::Vector4(0.0f, 1.0f, 0.0f, 0.0f),
        asdx::Vector3(0.0f, 2.0f, 0.0f),
        asdx::Vector3(0.0f, -1.0f, 0.0f));
    EXPECT_FLOAT_EQ(intersection.x, 0.0f);
    EXPECT_FLOAT_EQ(intersection.y, 0.0f);
    EXPECT_FLOAT_EQ(intersection.z, 0.0f);
}

TEST(MathTest, Matrix4x3Operations)
{
    const auto identity = asdx::Matrix4x3::CreateIdentity();
    const auto identityResult = asdx::Vector3::Transform(asdx::Vector3(1.0f, 2.0f, 3.0f), identity);
    EXPECT_FLOAT_EQ(identityResult.x, 1.0f);
    EXPECT_FLOAT_EQ(identityResult.y, 2.0f);
    EXPECT_FLOAT_EQ(identityResult.z, 3.0f);

    const auto translationOnly = asdx::Matrix4x3::CreateTranslation(10.0f, -20.0f, 30.0f);
    const auto translated = asdx::Vector3::Transform(asdx::Vector3(1.0f, 2.0f, 3.0f), translationOnly);
    EXPECT_FLOAT_EQ(translated.x, 11.0f);
    EXPECT_FLOAT_EQ(translated.y, -18.0f);
    EXPECT_FLOAT_EQ(translated.z, 33.0f);

    const asdx::Matrix4x3 matrix(
        2.0f, 0.0f, 0.0f,
        0.0f, 3.0f, 0.0f,
        0.0f, 0.0f, 4.0f,
        10.0f, -20.0f, 30.0f);
    const auto basisX = matrix.GetBasisX();
    const auto basisY = matrix.GetBasisY();
    const auto basisZ = matrix.GetBasisZ();
    EXPECT_FLOAT_EQ(basisX.x, 2.0f);
    EXPECT_FLOAT_EQ(basisY.y, 3.0f);
    EXPECT_FLOAT_EQ(basisZ.z, 4.0f);
    EXPECT_FLOAT_EQ(matrix.CalcScale().x, 2.0f);
    EXPECT_FLOAT_EQ(matrix.CalcScale().y, 3.0f);
    EXPECT_FLOAT_EQ(matrix.CalcScale().z, 4.0f);
    EXPECT_FLOAT_EQ(matrix.GetPosition().x, 10.0f);
    EXPECT_FLOAT_EQ(matrix.GetPosition().y, -20.0f);
    EXPECT_FLOAT_EQ(matrix.GetPosition().z, 30.0f);

    const auto expanded = asdx::Matrix4x4::FromMatrix4x3(matrix);
    EXPECT_FLOAT_EQ(expanded._11, 2.0f);
    EXPECT_FLOAT_EQ(expanded._22, 3.0f);
    EXPECT_FLOAT_EQ(expanded._33, 4.0f);
    EXPECT_FLOAT_EQ(expanded._41, 10.0f);
    EXPECT_FLOAT_EQ(expanded._44, 1.0f);
}

TEST(MathTest, ColorFunctions)
{
    const asdx::Vector3 color(1.0f, 0.5f, 0.0f);
    EXPECT_NEAR(asdx::LuminanceBT601(color), 0.5925f, 1.0e-4f);
    EXPECT_NEAR(asdx::LuminanceBT709(color), 0.5702f, 1.0e-4f);
    EXPECT_NEAR(asdx::LuminanceBT2020(color), 0.6017f, 1.0e-4f);

    const auto xy = asdx::CCT_To_xy(6504.0f);
    EXPECT_GT(xy.x, 0.2f);
    EXPECT_LT(xy.x, 0.4f);
    EXPECT_GT(xy.y, 0.2f);
    EXPECT_LT(xy.y, 0.5f);
    const auto xyz = asdx::CCT_To_XYZ(6504.0f);
    EXPECT_GT(xyz.y, 0.0f);
    EXPECT_FLOAT_EQ(asdx::CCT_To_XYZ(6504.0f, 2.0f).y, 2.0f);
}

TEST(MathTest, VectorAdditionalOperations)
{
    const asdx::Vector2 value(-2.0f, 3.0f);
    const auto abs = asdx::Vector2::Abs(value);
    const auto minimum = asdx::Vector2::Min(value, asdx::Vector2(1.0f, 1.0f));
    const auto maximum = asdx::Vector2::Max(value, asdx::Vector2(1.0f, 1.0f));
    EXPECT_FLOAT_EQ(abs.x, 2.0f);
    EXPECT_FLOAT_EQ(abs.y, 3.0f);
    EXPECT_FLOAT_EQ(minimum.x, -2.0f);
    EXPECT_FLOAT_EQ(minimum.y, 1.0f);
    EXPECT_FLOAT_EQ(maximum.x, 1.0f);
    EXPECT_FLOAT_EQ(maximum.y, 3.0f);
    EXPECT_FLOAT_EQ(asdx::Vector2::Reflect(asdx::Vector2(1.0f, -1.0f), asdx::Vector2(0.0f, 1.0f)).y, 1.0f);
    const auto barycentric = asdx::Vector2::Barycentric(
        asdx::Vector2(0.0f), asdx::Vector2(10.0f), asdx::Vector2(20.0f), 0.2f, 0.3f);
    EXPECT_FLOAT_EQ(barycentric.x, 8.0f);
    EXPECT_FLOAT_EQ(asdx::Vector2::SmoothStep(asdx::Vector2(0.0f), asdx::Vector2(10.0f), 0.0f).x, 0.0f);
    EXPECT_FLOAT_EQ(asdx::Vector2::SmoothStep(asdx::Vector2(0.0f), asdx::Vector2(10.0f), 1.0f).x, 10.0f);

    const auto triple = asdx::Vector3::VectorTriple(
        asdx::Vector3(1.0f, 0.0f, 0.0f), asdx::Vector3(0.0f, 1.0f, 0.0f), asdx::Vector3(1.0f, 1.0f, 0.0f));
    EXPECT_FLOAT_EQ(triple.z, 0.0f);
    const auto reflected = asdx::Vector3::Reflect(
        asdx::Vector3(1.0f, -1.0f, 0.0f), asdx::Vector3(0.0f, 1.0f, 0.0f));
    EXPECT_FLOAT_EQ(reflected.x, 1.0f);
    EXPECT_FLOAT_EQ(reflected.y, 1.0f);
    EXPECT_FLOAT_EQ(asdx::Vector3::FromRGB(255, 128, 0).x, 1.0f);
    EXPECT_NEAR(asdx::Vector3::FromRGB(255, 128, 0).y, 128.0f / 255.0f, 1.0e-6f);
}

TEST(MathTest, VectorTransformVariants)
{
    const auto translation = asdx::Matrix4x4::CreateTranslation(10.0f, 20.0f, 30.0f);
    const auto normal = asdx::Vector3::TransformNormal(asdx::Vector3(1.0f, 2.0f, 3.0f), translation);
    const auto coord = asdx::Vector3::TransformCoord(asdx::Vector3(1.0f, 2.0f, 3.0f), translation);
    EXPECT_FLOAT_EQ(normal.x, 1.0f);
    EXPECT_FLOAT_EQ(normal.y, 2.0f);
    EXPECT_FLOAT_EQ(normal.z, 3.0f);
    EXPECT_FLOAT_EQ(coord.x, 11.0f);
    EXPECT_FLOAT_EQ(coord.y, 22.0f);
    EXPECT_FLOAT_EQ(coord.z, 33.0f);

    const auto rotation = asdx::Quaternion::CreateFromAxisAngle(
        asdx::Vector3(0.0f, 0.0f, 1.0f), asdx::F_PIDIV2);
    const auto rotated = asdx::Vector3::Rotate(asdx::Vector3(1.0f, 0.0f, 0.0f), rotation);
    const auto restored = asdx::Vector3::InverseRotate(rotated, rotation);
    EXPECT_NEAR(rotated.y, 1.0f, 1.0e-6f);
    EXPECT_NEAR(restored.x, 1.0f, 1.0e-6f);
    EXPECT_NEAR(restored.y, 0.0f, 1.0e-6f);
}

TEST(MathTest, MatrixUtilityOperations)
{
    const auto matrix = asdx::Matrix4x4::CreateScale(2.0f, 3.0f, 4.0f);
    const auto inverse = asdx::Matrix4x4::Invert(matrix);
    const auto result = matrix * inverse;
    EXPECT_NEAR(result._11, 1.0f, 1.0e-6f);
    EXPECT_NEAR(result._22, 1.0f, 1.0e-6f);
    EXPECT_NEAR(result._33, 1.0f, 1.0e-6f);
    EXPECT_NEAR(result._44, 1.0f, 1.0e-6f);
    const auto transpose = asdx::Matrix4x4::Transpose(
        asdx::Matrix4x4::CreateTranslation(1.0f, 2.0f, 3.0f));
    EXPECT_FLOAT_EQ(transpose._14, 1.0f);
    EXPECT_FLOAT_EQ(transpose._24, 2.0f);
    EXPECT_FLOAT_EQ(transpose._34, 3.0f);
    EXPECT_FLOAT_EQ(asdx::Matrix4x4::Lerp(asdx::Matrix4x4::CreateIdentity(), matrix, 0.5f)._11, 1.5f);
    EXPECT_TRUE(asdx::Matrix4x4::IsIdentity(asdx::Matrix4x4::CreateIdentity()));
    EXPECT_FALSE(asdx::Matrix4x4::IsIdentity(matrix));
    EXPECT_FLOAT_EQ(asdx::Matrix4x4::Multiply(matrix, 2.0f)._11, 4.0f);
    const auto multipliedTranspose = asdx::Matrix4x4::MultiplyTranspose(
        matrix, asdx::Matrix4x4::CreateIdentity());
    EXPECT_FLOAT_EQ(multipliedTranspose._11, 2.0f);
}

TEST(MathTest, Vector4Operations)
{
    const asdx::Vector4 value(1.0f, 2.0f, 2.0f, 1.0f);
    EXPECT_FLOAT_EQ(value.LengthSq(), 10.0f);
    EXPECT_NEAR(value.Length(), std::sqrt(10.0f), 1.0e-6f);
    const auto normalized = asdx::Vector4::Normalize(value);
    EXPECT_NEAR(normalized.Length(), 1.0f, 1.0e-6f);
    const auto safe = asdx::Vector4::SafeNormalize(asdx::Vector4(), asdx::Vector4(1.0f));
    EXPECT_FLOAT_EQ(safe.x, 1.0f);
    EXPECT_FLOAT_EQ(safe.y, 1.0f);
    EXPECT_FLOAT_EQ(safe.z, 1.0f);
    EXPECT_FLOAT_EQ(safe.w, 1.0f);

    const auto barycentric = asdx::Vector4::Barycentric(
        asdx::Vector4(0.0f), asdx::Vector4(10.0f), asdx::Vector4(20.0f), 0.2f, 0.3f);
    EXPECT_FLOAT_EQ(barycentric.x, 8.0f);
    EXPECT_FLOAT_EQ(asdx::Vector4::SmoothStep(asdx::Vector4(0.0f), asdx::Vector4(1.0f), 0.0f).x, 0.0f);
    EXPECT_FLOAT_EQ(asdx::Vector4::SmoothStep(asdx::Vector4(0.0f), asdx::Vector4(1.0f), 1.0f).x, 1.0f);
}

TEST(MathTest, RandomHelper)
{
    EXPECT_EQ(asdx::RandomHelper::GetAsUint(0u, 10u, 20u), 10u);
    EXPECT_EQ(asdx::RandomHelper::GetAsUint(UINT32_MAX, 10u, 20u), 15u);
    EXPECT_EQ(asdx::RandomHelper::GetAsInt(0u), 0);
    EXPECT_EQ(asdx::RandomHelper::GetAsInt(UINT32_MAX), 0x7fffffff);
    EXPECT_EQ(asdx::RandomHelper::GetAsInt(0u, -5, 5), -5);
    EXPECT_EQ(asdx::RandomHelper::GetAsInt(UINT32_MAX, -5, 5), 2);
    EXPECT_FLOAT_EQ(asdx::RandomHelper::GetAsFloat(0u), 0.0f);
    EXPECT_LE(asdx::RandomHelper::GetAsFloat(UINT32_MAX), 1.0f);
    EXPECT_FLOAT_EQ(asdx::RandomHelper::GetAsFloat(0u, 2.0f, 4.0f), 2.0f);
}

TEST(MathTest, ColorConversionRoundTrip)
{
    const auto srgb = asdx::LinearToSRGB(asdx::Vector3(0.0f, 0.18f, 1.0f));
    EXPECT_FLOAT_EQ(srgb.x, 0.0f);
    EXPECT_NEAR(srgb.y, 0.461356f, 1.0e-5f);
    EXPECT_FLOAT_EQ(srgb.z, 1.0f);
    const auto linear = asdx::SRGBToLinear(asdx::Vector3(0.0f, 0.461356f, 1.0f));
    EXPECT_FLOAT_EQ(linear.x, 0.0f);
    EXPECT_NEAR(linear.y, 0.18f, 1.0e-5f);
    EXPECT_FLOAT_EQ(linear.z, 1.0f);

    const asdx::Vector4 color(0.25f, 0.5f, 0.75f, 0.4f);
    EXPECT_FLOAT_EQ(asdx::LinearToSRGB(color).w, color.w);
    EXPECT_FLOAT_EQ(asdx::SRGBToLinear(color).w, color.w);
}

TEST(MathTest, PackedColorFormats)
{
    const auto argb = asdx::Unorm4::FromARGB(0x12345678u);
    const auto bgra = asdx::Unorm4::FromBGRA(0x12345678u);
    const auto abgr = asdx::Unorm4::FromABGR(0x12345678u);
    EXPECT_EQ(argb.x, 0x34);
    EXPECT_EQ(argb.y, 0x56);
    EXPECT_EQ(argb.z, 0x78);
    EXPECT_EQ(argb.w, 0x12);
    EXPECT_EQ(bgra.x, 0x56);
    EXPECT_EQ(bgra.y, 0x34);
    EXPECT_EQ(bgra.z, 0x12);
    EXPECT_EQ(bgra.w, 0x78);
    EXPECT_EQ(abgr.x, 0x78);
    EXPECT_EQ(abgr.y, 0x56);
    EXPECT_EQ(abgr.z, 0x34);
    EXPECT_EQ(abgr.w, 0x12);
}

TEST(MathTest, ViewAndProjectionMatrices)
{
    const auto lookTo = asdx::Matrix4x4::CreateLookTo(
        asdx::Vector3(0.0f, 0.0f, -5.0f),
        asdx::Vector3(0.0f, 0.0f, 1.0f),
        asdx::Vector3(0.0f, 1.0f, 0.0f));
    EXPECT_NEAR(lookTo._11, -1.0f, 1.0e-6f);
    EXPECT_NEAR(lookTo._22, 1.0f, 1.0e-6f);
    EXPECT_NEAR(lookTo._33, -1.0f, 1.0e-6f);
    EXPECT_NEAR(lookTo._43, -5.0f, 1.0e-6f);

    const auto ortho = asdx::Matrix4x4::CreateOrthographic(2.0f, 2.0f, 1.0f, 11.0f);
    EXPECT_FLOAT_EQ(ortho._11, 1.0f);
    EXPECT_FLOAT_EQ(ortho._22, 1.0f);
    EXPECT_NEAR(ortho._33, -0.1f, 1.0e-6f);
    EXPECT_FLOAT_EQ(ortho._44, 1.0f);

    const auto perspective = asdx::Matrix4x4::CreatePerspective(2.0f, 2.0f, 1.0f, 11.0f);
    EXPECT_FLOAT_EQ(perspective._11, 1.0f);
    EXPECT_FLOAT_EQ(perspective._22, 1.0f);
    EXPECT_NE(perspective._34, 0.0f);
    EXPECT_FLOAT_EQ(perspective._44, 0.0f);
    const auto perspectiveFov = asdx::Matrix4x4::CreatePerspectiveFieldOfView(
        asdx::F_PIDIV2, 1.0f, 1.0f, 11.0f);
    EXPECT_NEAR(perspectiveFov._11, 1.0f, 1.0e-6f);
    EXPECT_NEAR(perspectiveFov._22, 1.0f, 1.0e-6f);
    EXPECT_NE(perspectiveFov._33, 0.0f);
    const auto offCenter = asdx::Matrix4x4::CreateOrthographicOffCenter(
        -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 11.0f);
    EXPECT_FLOAT_EQ(offCenter._11, 1.0f);
    EXPECT_FLOAT_EQ(offCenter._22, -1.0f);
    EXPECT_NE(offCenter._33, 0.0f);
}

TEST(MathTest, ReverseZProjectionMatrices)
{
    const auto perspective = asdx::Matrix4x4::CreatePerspectiveFieldOfViewReverseZ(
        asdx::F_PIDIV2, 1.0f, 1.0f);
    EXPECT_NEAR(perspective._11, 1.0f, 1.0e-6f);
    EXPECT_NEAR(perspective._22, 1.0f, 1.0e-6f);
    EXPECT_FLOAT_EQ(perspective._33, 0.0f);
    EXPECT_FLOAT_EQ(perspective._34, -1.0f);
    EXPECT_FLOAT_EQ(perspective._43, 1.0f);
    EXPECT_FLOAT_EQ(perspective._44, 0.0f);

    const auto orthographic = asdx::Matrix4x4::CreateOrthographicOffCenterReverseZ(
        -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 11.0f);
    EXPECT_FLOAT_EQ(orthographic._11, 1.0f);
    EXPECT_FLOAT_EQ(orthographic._22, -1.0f);
    EXPECT_FLOAT_EQ(orthographic._33, 0.1f);
    EXPECT_FLOAT_EQ(orthographic._43, 1.1f);
    EXPECT_FLOAT_EQ(orthographic._44, 1.0f);

    const auto nearPoint = asdx::Vector3::TransformCoord(
        asdx::Vector3(0.0f, 0.0f, -1.0f), orthographic);
    const auto farPoint = asdx::Vector3::TransformCoord(
        asdx::Vector3(0.0f, 0.0f, -11.0f), orthographic);
    EXPECT_FLOAT_EQ(nearPoint.z, 1.0f);
    EXPECT_FLOAT_EQ(farPoint.z, 0.0f);
}

TEST(MathTest, ColorTemperatureConversions)
{
    const auto xyz = asdx::CCT_To_XYZ(6504.0f, 2.0f);
    const auto bt601 = asdx::CCT_To_BT601(6504.0f, 2.0f);
    const auto bt709 = asdx::CCT_To_BT709(6504.0f, 2.0f);
    const auto bt2020 = asdx::CCT_To_BT2020(6504.0f, 2.0f);
    EXPECT_FLOAT_EQ(xyz.y, 2.0f);
    EXPECT_TRUE(std::isfinite(bt601.x));
    EXPECT_TRUE(std::isfinite(bt601.y));
    EXPECT_TRUE(std::isfinite(bt601.z));
    EXPECT_TRUE(std::isfinite(bt709.x));
    EXPECT_TRUE(std::isfinite(bt709.y));
    EXPECT_TRUE(std::isfinite(bt709.z));
    EXPECT_TRUE(std::isfinite(bt2020.x));
    EXPECT_TRUE(std::isfinite(bt2020.y));
    EXPECT_TRUE(std::isfinite(bt2020.z));
}
