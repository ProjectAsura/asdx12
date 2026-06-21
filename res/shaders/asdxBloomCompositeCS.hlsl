//-----------------------------------------------------------------------------
// File : asdxBloomCompositeCS.hlsl
// Desc : Compute Shader For Bloom.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxComputeUtil.hlsli"
#include "asdxSamplers.hlsli"

//-----------------------------------------------------------------------------
// Resources
//-----------------------------------------------------------------------------
Texture2D<float4>   ColorMap    : register(t0);
RWTexture2D<float4> FilteredMap : register(u0);


///////////////////////////////////////////////////////////////////////////////
// CbParam constant buffers.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbParam : register(b0)
{
    uint    SrcResolution;
    uint    DstResolution;
    uint2   Reserved;
};

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
[numthreads(8, 8, 8)]
void main(uint3 dispatchId : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
    uint2 remapId = RemapLane8x8(dispatchId.xy, groupIndex);
    uint dstW = DstResolution & 0xFFFF;
    uint dstH = (DstResolution >> 16) & 0xFFFF;

    if (any(remapId > uint2(dstW, dstH)))
        return;

    uint srcW = SrcResolution & 0xFFFF;
    uint srcH = (SrcResolution >> 16) & 0xFFFF;

    float2 invSrcSize = float2(1.0f / float(srcW), 1.0f / float(srcW));
    float2 uv = float2(remapId + 0.5f.xx) * invSrcSize;

    float4 result = 0.0f.xxxx;

    result += ColorMap.SampleLevel(LinearClamp, uv + float2(-0.5f, -0.5f) * invSrcSize, 0.0f);
    result += ColorMap.SampleLevel(LinearClamp, uv + float2( 0.5f, -0.5f) * invSrcSize, 0.0f);
    result += ColorMap.SampleLevel(LinearClamp, uv + float2(-0.5f,  0.5f) * invSrcSize, 0.0f);
    result += ColorMap.SampleLevel(LinearClamp, uv + float2( 0.5f,  0.5f) * invSrcSize, 0.0f);
 
    result *= 0.025f;

    FilteredMap[remapId] = result;
}
