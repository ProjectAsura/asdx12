//-----------------------------------------------------------------------------
// File : asdxStarCS.hlsl
// Desc : Compute Shader For Star.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxSamplers.hlsli"
#include "asdxComputeUtil.hlsli"


///////////////////////////////////////////////////////////////////////////////
// Param constant buffers.
///////////////////////////////////////////////////////////////////////////////
cbuffer Param : register(b0)
{
    float4 Offset[4];
    float4 Weight[8];
    uint   Resolution;
    uint3  Reserved;
}

//-----------------------------------------------------------------------------
// Resources.
//-----------------------------------------------------------------------------
Texture2D<float4>   ColorMap  : register(t0);
RWTexture2D<float4> OutputMap : register(u0);

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
    uint2 remapId = RemapLane8x8(dispatchId.xy, groupIndex);
    uint2 dstSize = GetTargetSize(Resolution);

    if (any(remapId >= dstSize))
        return;

    float2 uv = (remapId + 0.5f.xx) / float2(dstSize);
 
    const float2 offset[8] = (float2[8])Offset;

    float4 result = 0.0f.xxxx;
    [unroll]
    for(int i=0; i<8; ++i)
        result += ColorMap.SampleLevel(LinearClamp, uv + offset[i].xy, 0.0f) * Weight[i];

    result.a = 1.0f;

    OutputMap[remapId] = max(result, 0.0f.xxxx);
}
