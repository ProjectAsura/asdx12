//-----------------------------------------------------------------------------
// File : asdxSmaaCalcBlendWeightCS.hlsl
// Desc : Calcuate Blend Weight For SMAA.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxSmaa.hlsli"

//-----------------------------------------------------------------------------
// Resources
//-----------------------------------------------------------------------------
Texture2D           EdgeMap   : register(t0);   // float2
Texture2D           AreaMap   : register(t1);   // float2
Texture2D           SearchMap : register(t2);   // float
RWTexture2D<float4> BlendMap  : register(u0);

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
    uint2 remapId = RemapLane8x8(dispatchId.xy, groupIndex);
    if (any(remapId >= uint2(Size.zw)))
        return;

    float2 uv = (remapId + 0.5f.xx) * Size.xy;

    float4 offset[3];
    float2 pixCoord;
    SMAABlendingWeightCalculationVS(uv, pixCoord, offset);
    float4 result = SMAABlendingWeightCalculationPS(
        uv, 
        pixCoord,
        offset,
        EdgeMap,
        AreaMap,
        SearchMap,
        0.0f.xxxx);

    BlendMap[remapId] = result;
}
