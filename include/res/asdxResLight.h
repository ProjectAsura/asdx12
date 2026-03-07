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

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static constexpr float kLightUnitWriteScale = 0.001f;   //!< レンダーターゲット書き込み時のライト単位スケール値 (= 1.0/1000.0).
static constexpr float kLightUnitReadScale  = 1000.0f;  //!< レンダーターゲット読み込み時のライト単位スケール値.


///////////////////////////////////////////////////////////////////////////////
// ResPointLight structure
///////////////////////////////////////////////////////////////////////////////
struct ResPointLight
{
    Vector3     Position;       //!< 位置座標.
    Vector3     Color;          //!< カラー.
    float       Radius;         //!< 半径.
    float       Itensity;       //!< 強度(カンデラ単位).
};

///////////////////////////////////////////////////////////////////////////////
// ResSpotLight structure
///////////////////////////////////////////////////////////////////////////////
struct ResSpotLight
{
    Vector3     Position;       //!< 位置座標.
    Vector3     Direction;      //!< 照射方向.
    Vector3     Color;          //!< カラー.
    float       Radius;         //!< 半径.
    float       Intensity;      //!< 強度(カンデラ単位)
    float       InnterAngle;    //!< 内角(ラジアン単位).
    float       OuterAngle;     //!< 外角(ラジアン単位).
};

///////////////////////////////////////////////////////////////////////////////
// ResDirectionalLight structure
///////////////////////////////////////////////////////////////////////////////
struct ResDirectionalLight
{
    Vector3     Direction;      //!< 照射方向.
    Vector3     Color;          //!< カラー.
    float       Intensity;      //!< 強度(ルクス単位).
};

///////////////////////////////////////////////////////////////////////////////
// ResImageBasedLight structure
///////////////////////////////////////////////////////////////////////////////
struct ResImageBasedLight
{
    StringView  Path;           //!< ファイルパス.
    float       Intensity;      //!< 強度(ルクス単位).
};

} // namespace asdx
