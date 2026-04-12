//-----------------------------------------------------------------------------
// File : asdxColorFilterPS.hlsl
// Desc : Pixel Shader for Color Filter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxComputeUtil.hlsli"
#include "asdxSamplers.hlsli"


///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD0;
};

///////////////////////////////////////////////////////////////////////////////
// CbColorFilter constant buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbColorFilter : register(b0)
{
    uint2       TargetSize;
    float2      InvTargetSize;
    float4x4    ColorMatrix;
};

//-----------------------------------------------------------------------------
// Resources.
//-----------------------------------------------------------------------------
Texture2D<float4> Input : register(t0);

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET0
{
    float4 texel = Input.Sample(LinearClamp, input.TexCoord);
    return mul(ColorMatrix, texel);
}
