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
    float2  Offset;
    float2  Size;
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

    output += BlurSample(input.TexCoord, Offset * 1.0f, Weights0.x);
    output += BlurSample(input.TexCoord, Offset * 3.0f, Weights0.y);
    output += BlurSample(input.TexCoord, Offset * 5.0f, Weights0.z);
    output += BlurSample(input.TexCoord, Offset * 7.0f, Weights0.w);

    output += BlurSample(input.TexCoord, Offset *  9.0f, Weights1.x);
    output += BlurSample(input.TexCoord, Offset * 11.0f, Weights1.y);
    output += BlurSample(input.TexCoord, Offset * 13.0f, Weights1.z);
    output += BlurSample(input.TexCoord, Offset * 15.0f, Weights1.w);

    return output;
}
