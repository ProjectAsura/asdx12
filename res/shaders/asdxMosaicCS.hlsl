//-----------------------------------------------------------------------------
// File : asdxMosaicCS.hlsl
// Desc : Compute Shader For Mosaic Effect.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxComputeUtil.hlsli"
#include "asdxSamplers.hlsli"


///////////////////////////////////////////////////////////////////////////////
// Param constant buffers.
///////////////////////////////////////////////////////////////////////////////
cbuffer Param : register(b0)
{
    float Block;
    uint  DstResolution;
    uint2 Reserved;
};

//-----------------------------------------------------------------------------
// Resources
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
    uint2 dstSize = GetTargetSize(DstResolution);
    if (any(remapId >= dstSize))
        return;

    float2 uv = (remapId + 0.5f.xx) / float2(dstSize);
    float2 st = floor(Block * uv + 0.5f.xx) / Block;
    float4 color = ColorMap.SampleLevel(LinearWrap, st, 0.0f);
    color.a = 1.0f;

    OutputMap[remapId] = color;
}
