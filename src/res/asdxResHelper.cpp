//-----------------------------------------------------------------------------
// File : asdxResHelper.cpp
// Desc : ResType Helpers.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "ResTypes_generated.h"
#include <res/asdxResHelper.h>


namespace asdx {

res::Float2 ToFloat2(const Vector2& value)
{ return res::Float2(value.x, value.y); }

res::Float3 ToFloat3(const Vector3& value)
{ return res::Float3(value.x, value.y, value.z); }

res::Float4 ToFloat4(const Vector4& value)
{ return res::Float4(value.x, value.y, value.z, value.w); }

res::Quaternion ToQuaternion(const Quaternion& value)
{ return res::Quaternion(value.x, value.y, value.z, value.w); }

res::Uint2 ToUint2(const Uint2& value)
{ return res::Uint2(value.x, value.y); }

res::Uint3 ToUint3(const Uint3& value)
{ return res::Uint3(value.x, value.y, value.z); }

res::Uint4 ToUint4(const Uint4& value)
{ return res::Uint4(value.x, value.y, value.z, value.w); }

res::Unorm2 ToUnorm2(const Unorm2& value)
{ return res::Unorm2(value.x, value.y); }

res::Unorm3 ToUnorm3(const Unorm3& value)
{ return res::Unorm3(value.x, value.y, value.z); }

res::Unorm4 ToUnorm4(const Unorm4& value)
{ return res::Unorm4(value.x, value.y, value.z, value.w); }

res::Float3x4 ToFloat3x4(const Transform3x4& value)
{
    return res::Float3x4(
        value._11, value._12, value._13, value._14,
        value._21, value._22, value._23, value._24,
        value._31, value._32, value._33, value._34);
}

res::Float4x4 ToFloat4x4(const Matrix& value)
{
    return res::Float4x4(
        value._11, value._12, value._13, value._14,
        value._21, value._22, value._23, value._24,
        value._31, value._32, value._33, value._34,
        value._41, value._42, value._43, value._44);
}

res::BoundingBox ToBox(const BoundingBox3& value)
{
    return res::BoundingBox(
        res::Float3(value.Mini.x, value.Mini.y, value.Mini.z),
        res::Float3(value.Maxi.x, value.Maxi.y, value.Maxi.z));
}

res::BoundingSphere ToSphere(const BoundingSphere3& value)
{
    return res::BoundingSphere(
        res::Float3(value.Center.x, value.Center.y, value.Center.z),
        value.Radius);
}

Vector2 FromFloat2(const res::Float2& value)
{ return Vector2(value.X(), value.Y()); }

Vector3 FromFloat3(const res::Float3& value)
{ return Vector3(value.X(), value.Y(), value.Z()); }

Vector4 FromFloat4(const res::Float4& value)
{ return Vector4(value.X(), value.Y(), value.Z(), value.W()); }

Quaternion FromQuaternion(const res::Quaternion& value)
{ return Quaternion(value.X(), value.Y(), value.Z(), value.W()); }

Uint2 FromUint2(const res::Uint2& value)
{ return Uint2(value.X(), value.Y()); }

Uint3 FromUint3(const res::Uint3& value)
{ return Uint3(value.X(), value.Y(), value.Z()); }

Uint4 FromUint4(const res::Uint4& value)
{ return Uint4(value.X(), value.Y(), value.Z(), value.W()); }

Transform3x4 FromFloat3x4(const res::Float3x4& value)
{
    return Transform3x4(
        value.M11(), value.M12(), value.M13(), value.M14(),
        value.M21(), value.M22(), value.M23(), value.M24(),
        value.M31(), value.M32(), value.M33(), value.M34());
}

Matrix FromFloat4x4(const res::Float4x4& value)
{
    return Matrix(
        value.M11(), value.M12(), value.M13(), value.M14(),
        value.M21(), value.M22(), value.M23(), value.M24(),
        value.M31(), value.M32(), value.M33(), value.M34(),
        value.M41(), value.M42(), value.M43(), value.M44());
}

BoundingBox3 FromBox(const res::BoundingBox& value)
{
    return BoundingBox3(
        Vector3(value.Min().X(), value.Min().Y(), value.Min().Z()),
        Vector3(value.Max().X(), value.Max().Y(), value.Max().Z()));
}

BoundingSphere3 FromSphere(const res::BoundingSphere& value)
{
    return BoundingSphere3(
        Vector3(value.Center().X(), value.Center().Y(), value.Center().Z()),
        value.Radius());
}

} // namespace asdx
