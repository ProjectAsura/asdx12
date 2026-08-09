//-----------------------------------------------------------------------------
// File : asdxSmaaNeighborBlendingCS.hlsl
// Desc : Blend with Neighbors for SMAA.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxSmaa.hlsli"

//-----------------------------------------------------------------------------
// Resources
//-----------------------------------------------------------------------------
Texture2D           ColorMap  : register(t0);
Texture2D           BlendMap  : register(t1);
RWTexture2D<float4> OutputMap : register(u0);

//-----------------------------------------------------------------------------
//      メインエントリーポイントです
//-----------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
    uint2 remapId = RemapLane8x8(dispatchId.xy, groupIndex);
    if (any(remapId >= uint2(Size.zw)))
        return;

    float2 uv = (remapId + 0.5f.xx) * Size.xy;

    float4 offset;
    SMAANeighborhoodBlendingVS(uv, offset);
    float4 result = SMAANeighborhoodBlendingPS(uv, offset, ColorMap, BlendMap);

    OutputMap[remapId] = result;
}
