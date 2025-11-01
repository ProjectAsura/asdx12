//-----------------------------------------------------------------------------
// File : asdxResLight.h
// Desc : Light Resource.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxMath.h>
#include <fnd/asdxStringView.h>


namespace asdx {

///////////////////////////////////////////////////////////////////////////////
// ResPointLight structure
///////////////////////////////////////////////////////////////////////////////
struct ResPointLight
{
    Vector3     Position;       //!< 位置座標.
    float       Radius;         //!< 半径.
    Vector4     Color;          //!< カラー.
};

///////////////////////////////////////////////////////////////////////////////
// ResSpotLight structure
///////////////////////////////////////////////////////////////////////////////
struct ResSpotLight
{
    Vector3     Position;       //!< 位置座標.
    float       InnterAngle;    //!< 内角(ラジアン単位).
    float       OuterAngle;     //!< 外角(ラジアン単位).
    Vector3     Direction;      //!< 照射方向.
    Vector4     Color;          //!< カラー.
};

///////////////////////////////////////////////////////////////////////////////
// ResDirectionalLight structure
///////////////////////////////////////////////////////////////////////////////
struct ResDirectionalLight
{
    Vector3     Direction;      //!< 照射方向.
    Vector4     Color;          //!< カラー.
};

///////////////////////////////////////////////////////////////////////////////
// ResImageBasedLight structure
///////////////////////////////////////////////////////////////////////////////
struct ResImageBasedLight
{
    StringView  Path;           //!< テクスチャファイルパス.
    Vector4     Color;          //!< カラー.
};

} // namespace asdx
