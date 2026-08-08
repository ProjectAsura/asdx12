//-----------------------------------------------------------------------------
// File : asdxRadialBlurCS.hlsl
// Desc : Compute Shader for Radial Blur.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxComputeUtil.hlsli"
#include "asdxRandom.hlsli"
#include "asdxSamplers.hlsli"


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
    uint SrcResolution;
    uint DstResolution;
};

//-----------------------------------------------------------------------------
// Resources.
//-----------------------------------------------------------------------------
Texture2D<float4>   Input   : register(t0);
RWTexture2D<float4> Output  : register(u0);

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
    uint2 dstSize    = GetTargetSize(DstResolution);
    if (any(remappedId >= dstSize))
    { return; }
 
    float2 inputTexCoord = remappedId / float2(dstSize);

    float4 output = float4(0.0f, 0.0f, 0.0f, 0.0f);
    const float2 center = float2(Center.x, 1.0f - Center.y);

    // サンプルオフセットを計算.
    float2 uvOffset = (center - inputTexCoord) * (Strength / float2(dstSize));
    uvOffset.x *= (float(dstSize.x) / float(dstSize.y)); // アスペクト比を考慮.
 
    // サンプル重み.
    const float kWeight = 1.0f / SampleCount;
 
    // Stochastic Sampling
    const float kNoise  = InterleavedGradientNoise(float2(remappedId));

    [loop]
    for(uint i=0; i<SampleCount; ++i)
    {
        float2 uv = inputTexCoord + uvOffset * i * (1.0f + kNoise);
        output += Input.SampleLevel(LinearClamp, uv, 0.0f) * kWeight;
    }

    Output[remappedId] = output;
}
