//-----------------------------------------------------------------------------
// File : asdxRadialBlurPS.hlsl
// Desc : Pixel Shader For Radial Blur.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxRandom.hlsli"
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
// CbParam constant buffers.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbParam0 : register(b0)
{
    float2 Center;
    float  Strength;
    uint   SampleCount;
};

///////////////////////////////////////////////////////////////////////////////
// CbParam1 constant buffers.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbParam1 : register(b1)
{
    uint2   InputSize;
    uint2   OutputSize;
};

//-----------------------------------------------------------------------------
// Resources.
//-----------------------------------------------------------------------------
Texture2D ColorMap : register(t0);

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const VSOutput input) : SV_TARGET0
{
    float4 output = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float2 center = float2(Center.x, 1.0f - Center.y);

    // サンプルオフセットを計算.
    float2 uvOffset = (center - input.TexCoord) * (Strength / float2(InputSize));
    uvOffset.x *= (float(InputSize.x) / float(InputSize.y));   // アスペクト比を考慮.
 
    // サンプル重み.
    const float kWeight = 1.0f / SampleCount;

    // Stochastic Sampling.
    const float kNoise = InterleavedGradientNoise(input.Position.xy);

    [loop]
    for(uint i=0; i<SampleCount; ++i)
    {
        float2 uv = input.TexCoord + uvOffset * i * (1.0f + kNoise);
        output += ColorMap.SampleLevel(LinearClamp, uv, 0.0f) * kWeight;
    }

    return output;
}