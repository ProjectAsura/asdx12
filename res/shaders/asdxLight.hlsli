//-----------------------------------------------------------------------------
// File : asdxLight.hlsli
// Desc : Light.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#ifndef ASDX_LIGHT_HLSLI
#define ASDX_LIGHT_HLSLI

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxMath.hlsli"


//-----------------------------------------------------------------------------
//      カンデラからルクスに変換します.
//-----------------------------------------------------------------------------
float CandelaToLux(float cd, float distanceSq, float cosAngle)
{ return (cd * cosAngle) / distanceSq; }

//-----------------------------------------------------------------------------
//      ルクスからカンデラに変換します.
//-----------------------------------------------------------------------------
float LuxToCandela(float lux, float distanceSq, float cosAngle)
{ return (lux * distanceSq) / cosAngle; }

//-----------------------------------------------------------------------------
//      カンデラからルーメンに変換します.
//-----------------------------------------------------------------------------
float CandelaToLumen(float cd, float solidAngle)
{ return cd * solidAngle; }

//-----------------------------------------------------------------------------
//      ルーメンからカンデラに変換します.
//-----------------------------------------------------------------------------
float LumenToCandela(float lumen, float solidAngle)
{ return lumen / max(solidAngle, 1e-4f); }

//-----------------------------------------------------------------------------
//      ルクスからルーメンに変換します.
//-----------------------------------------------------------------------------
float LuxToLumen(float lux, float distanceSq, float cosAngle)
{
    float cd = LuxToCandela(lux, distanceSq, cosAngle);
    float omega = F_2PI * (1.0f - cosAngle);
    return CandelaToLumen(cd, omega);
}

//-----------------------------------------------------------------------------
//      ルーメンからルクスに変換します.
//-----------------------------------------------------------------------------
float LumenToLux(float lumen, float distanceSq, float cosAngle)
{
    float omega = F_2PI * (1.0f - cosAngle);
    float cd = LumenToCandela(lumen, omega);
    return CandelaToLux(cd, distanceSq, cosAngle);
}

//-----------------------------------------------------------------------------
//      ポイントライトの照度値(lux)を計算します.
//-----------------------------------------------------------------------------
float CalcPointIlluminance(float lumen, float distanceSq)
{ return LumenToLux(lumen, distanceSq, -1.0f); }

//-----------------------------------------------------------------------------
//      スポットライトの照度値(lux)を計算します.
//-----------------------------------------------------------------------------
float CalcSpotIlluminance(float lumen, float distanceSq, float cosOuterAngle)
{ return LumenToLux(lumen, distanceSq, cosOuterAngle); }


#endif//ASDX_LIGHT_HLSLI
