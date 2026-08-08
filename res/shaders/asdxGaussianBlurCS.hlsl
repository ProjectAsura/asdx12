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
    float4  Offset0;
    float4  Offset1;
    uint    SrcResolution;
    uint    DstResolution;
    uint    Flags;
    uint    Reserved;
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
    uint2 dstSize = GetTargetSize(DstResolution);
 
    if (any(remappedId >= dstSize))
    { return; }

    uint2 srcSize = GetTargetSize(SrcResolution);
    float2 dir = (Flags == 0)
        ? float2(1.0f / float(srcSize.x), 0.0f)
        : float2(0.0f, 1.0f / float(srcSize.y));

    float2 uv = (remappedId + 0.5f.xx) / float2(dstSize);

    float4 output = 0.0f.xxxx;
    output += BlurSample(uv, dir * Offset0.x, Weights0.x);
    output += BlurSample(uv, dir * Offset0.y, Weights0.y);
    output += BlurSample(uv, dir * Offset0.z, Weights0.z);
    output += BlurSample(uv, dir * Offset0.w, Weights0.w);

    output += BlurSample(uv, dir * Offset1.x, Weights1.x);
    output += BlurSample(uv, dir * Offset1.y, Weights1.y);
    output += BlurSample(uv, dir * Offset1.z, Weights1.z);
    output += BlurSample(uv, dir * Offset1.w, Weights1.w);

    OutputMap[remappedId] = output;
}
