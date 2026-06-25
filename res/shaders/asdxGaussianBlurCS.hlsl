//-----------------------------------------------------------------------------
// File : asdxGaussianBlurCS.hlsl
// Desc : Compute Shader For Gaussian Blur Effect.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxSamplers.hlsli"
#include "asdxComputeUtil.hlsli"


///////////////////////////////////////////////////////////////////////////////
// CbParam structure
///////////////////////////////////////////////////////////////////////////////
cbuffer CbParam : register(b0)
{
    float4  Weights0;
    float4  Weights1;
    float2  Offset;
    uint2   Size;
};

//-----------------------------------------------------------------------------
// Resources
//-----------------------------------------------------------------------------
Texture2D           ColorMap  : register(t0);
RWTexture2D<float4> OutputMap : register(u0);

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
[numthreads(8, 8, 1)]
void main
(
    uint3 dispatchId : SV_DispatchThreadID,
    uint  groupIndex : SV_GroupIndex
)
{
    uint2 remappedId = RemapLane8x8(dispatchId.xy, groupIndex);
    if (any(remappedId >= Size))
    { return; }

    float2 uv = remappedId / float2(Size);

    float4 output = 0.0f.xxxx;
    output += BlurSample(uv, Offset * 0.5f, Weights0.x);
    output += BlurSample(uv, Offset * 2.5f, Weights0.y);
    output += BlurSample(uv, Offset * 4.5f, Weights0.z);
    output += BlurSample(uv, Offset * 6.5f, Weights0.w);

    output += BlurSample(uv, Offset *  8.5f, Weights1.x);
    output += BlurSample(uv, Offset * 10.5f, Weights1.y);
    output += BlurSample(uv, Offset * 12.5f, Weights1.z);
    output += BlurSample(uv, Offset * 14.5f, Weights1.w);

    OutputMap[remappedId] = output;
}
