//-----------------------------------------------------------------------------
// File : asdxResHelper.h
// Desc : ResType Helper.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxMath.h>

// NOTE : ResTypes_generated.h をこのファイルよりも先にインクルードしておいてください.
// 
//        ResTypes_generated.hが公開ヘッダではないのと，flatbuffersを公開したくないため.
//        さらにアプリ側ではインクルードしたい場面があるため, このヘッダを公開化する必要がある.
#ifndef FLATBUFFERS_GENERATED_RESTYPES_ASDX_RES_H_
#error "ResTypes_gnerated.hがインクルードされていません."
#endif

namespace asdx {

res::Float2 ToFloat2(const Vector2& value);
res::Float3 ToFloat3(const Vector3& value);
res::Float4 ToFloat4(const Vector4& value);
res::Quaternion ToQuaternion(const Quaternion& value);

res::Uint2 ToUint2(const Uint2& value);
res::Uint3 ToUint3(const Uint3& value);
res::Uint4 ToUint4(const Uint4& vlaue);

res::Unorm2 ToUnorm2(const Unorm2& value);
res::Unorm3 ToUnorm3(const Unorm3& value);
res::Unorm4 ToUnorm4(const Unorm4& value);

res::Float3x4 ToFloat3x4(const Transform4x3& value);
res::Float4x4 ToFloat4x4(const Matrix& value);

res::BoundingBox ToBox(const BoundingBox3& value);
res::BoundingSphere ToSphere(const BoundingSphere3& value);

Vector2 FromFloat2(const res::Float2& value);
Vector3 FromFloat3(const res::Float3& value);
Vector4 FromFloat4(const res::Float4& value);
Quaternion FromQuaternion(const res::Quaternion& value);

Uint2 FromUint2(const res::Uint2& value);
Uint3 FromUint3(const res::Uint3& value);
Uint4 FromUint4(const res::Uint4& value);

Transform4x3 FromFloat3x4(const res::Float3x4& value);
Matrix FromFloat4x4(const res::Float4x4& value);

BoundingBox3 FromBox(const res::BoundingBox& value);
BoundingSphere3 FromSphere(const res::BoundingSphere& value);

} // namespace asdx
