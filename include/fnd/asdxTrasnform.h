//-----------------------------------------------------------------------------
// File : asdxTransform.h
// Desc : Transform Data.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxMath.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// Transform structure
///////////////////////////////////////////////////////////////////////////////
struct Transform
{
    Vector3     Position = Vector3(0.0f, 0.0f, 0.0f);
    Vector3     Scale    = Vector3(1.0f, 1.0f, 1.0f);
    Quaternion  Rotation = Quaternion::CreateIdentity();
};

//-----------------------------------------------------------------------------
//! @brief      行列を計算します.
//! 
//! @param[in]      value       トランスフォームデータ.
//! @return     計算した行列を返却します.
//-----------------------------------------------------------------------------
inline Matrix4x4 CalcMatrix4x4(const Transform& value)
{
    Matrix4x4 result;
    result  = Matrix4x4::CreateScale(value.Scale);
    result *= Matrix4x4::CreateFromQuaternion(value.Rotation);
    result  = Matrix4x4::AppendTranslation(result, value.Position);
    return result;
}

//-----------------------------------------------------------------------------
//! @brief      4x3行列を計算します.
//! 
//! @param[in]      value       トランスフォームデータ.
//! @return     計算した4x3行列を返却します.
//-----------------------------------------------------------------------------
inline Matrix4x3 CalcMatrix4x3(const Transform& value)
{
    Matrix4x3 result;
    result  = Matrix4x3::CreateScale(value.Scale);
    result *= Matrix4x3::CreateFromQuaternion(value.Rotation);
    result  = Matrix4x3::AppendTranslation(result, value.Position);
    return result;
}

} // namespace asdx
