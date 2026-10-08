//-----------------------------------------------------------------------------
// File : asdxGaussianBlurPS.hlsl
// Desc : Pixel Shader For Gaussian Blur Effect.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxSamplers.hlsli"
#include "asdxComputeUtil.hlsli"


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
    float4  Weights[2];
    float4  Offsets[2];
    uint    SrcResolution;
    uint    DstResolution;
    uint    Flags;
    uint    Reserved;
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
    float2 uv = input.TexCoord;
    
    uint2 srcSize = GetTargetSize(SrcResolution);
    float2 dir = (Flags == 0)
        ? float2(1.0f / float(srcSize.x), 0.0f)
        : float2(0.0f, 1.0f / float(srcSize.y));

    const float offsets[8] = (float[8]) Offsets;
    const float weights[8] = (float[8]) Weights;

    float4 output = 0.0f.xxxx;
    [unroll]
    for (int i = 0; i < 8; ++i)
        output += BlurSample(uv, dir * offsets[i], weights[i]);

    return output;
}
