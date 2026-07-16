//-----------------------------------------------------------------------------
// File : asdxGaussianBlurPS.hlsl
// Desc : Pixel Shader For Gaussian Blur Effect.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxSamplers.hlsli"

///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

///////////////////////////////////////////////////////////////////////////////
// CbParam structure
///////////////////////////////////////////////////////////////////////////////
cbuffer CbParam : register(b0)
{
    float4  Weights0;
    float4  Weights1;
    float4  Offset0;
    float4  Offset1;
    uint    Resolution;
    uint    Flags;
};

//-----------------------------------------------------------------------------
// Resources
//-----------------------------------------------------------------------------
Texture2D ColorMap : register(t0);

//-----------------------------------------------------------------------------
//      ブラーサンプリングを行います.
//-----------------------------------------------------------------------------
float4 BlurSample(float2 uv, float2 offset, float weight)
{
    return (ColorMap.SampleLevel(LinearClamp, uv - offset, 0.0f) 
          + ColorMap.SampleLevel(LinearClamp, uv + offset, 0.0f)) * weight;
}

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET0
{
    float4 output = 0.0f.xxxx;

    uint sizeX = Resolution & 0xFFFF;
    uint sizeY = (Resolution >> 16) & 0xFFFF;
 
    float2 dir = (Flags == 0) 
        ? float2(1.0f / float(sizeX), 0.0f) 
        : float2(0.0f, 1.0f / float(sizeY));

    output += BlurSample(input.TexCoord, dir * Offset0.x, Weights0.x);
    output += BlurSample(input.TexCoord, dir * Offset0.y, Weights0.y);
    output += BlurSample(input.TexCoord, dir * Offset0.z, Weights0.z);
    output += BlurSample(input.TexCoord, dir * Offset0.w, Weights0.w);

    output += BlurSample(input.TexCoord, dir * Offset1.x, Weights1.x);
    output += BlurSample(input.TexCoord, dir * Offset1.y, Weights1.y);
    output += BlurSample(input.TexCoord, dir * Offset1.z, Weights1.z);
    output += BlurSample(input.TexCoord, dir * Offset1.w, Weights1.w);

    return output;
}
