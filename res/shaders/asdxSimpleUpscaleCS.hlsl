//-----------------------------------------------------------------------------
// File : asdxSimpleUpscaleCS.hlsl
// Desc : Compute Shader For Upscale.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxComputeUtil.hlsli"
#include "asdxSamplers.hlsli"


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
// Resources
//-----------------------------------------------------------------------------
Texture2D<float4>   ColorMap    : register(t0);
RWTexture2D<float4> OutputMap   : register(u0);


//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
    uint2 remapId = RemapLane8x8(dispatchId.xy, groupIndex);
    uint2 dstSize;
    dstSize.x = DstResolution & 0xFFFF;
    dstSize.y = (DstResolution >> 16) & 0xFFFF;

    if (any(remapId >= dstSize))
        return;

    float2 invSrcSize;
    invSrcSize.x = 1.0f / float(SrcResolution & 0xFFFF);
    invSrcSize.y = 1.0f / float((SrcResolution >> 16) & 0xFFFF);
 
    float2 uv = float2(remapId + 0.5f.xx) / float2(dstSize);
    float4 result = 0.0f.xxxx;

    // テントフィルタ.
    result += ColorMap.SampleLevel(LinearClamp, uv + float2(-0.5f, -0.5f) * invSrcSize, 0.0f);
    result += ColorMap.SampleLevel(LinearClamp, uv + float2( 0.5f, -0.5f) * invSrcSize, 0.0f);
    result += ColorMap.SampleLevel(LinearClamp, uv + float2(-0.5f,  0.5f) * invSrcSize, 0.0f);
    result += ColorMap.SampleLevel(LinearClamp, uv + float2( 0.5f,  0.5f) * invSrcSize, 0.0f); 
    result *= 0.25f;

    OutputMap[remapId] = float4(result.rgb, 1.0f);
}
