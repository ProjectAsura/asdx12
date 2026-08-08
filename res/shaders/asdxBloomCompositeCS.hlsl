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

///////////////////////////////////////////////////////////////////////////////
// CbParam constant buffers.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbParam : register(b0)
{
    uint    SrcResolution;  //!< 入力解像度.
    uint    DstResolution;  //!< 出力解像度.
    uint2   Reserved;       //!< 予約領域.
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
    uint2 dstSize = GetTargetSize(DstResolution);

    if (any(remapId >= dstSize))
        return;
 
    float2 invDstSize = 1.0f.xx / float2(dstSize);
    float2 uv = float2(remapId + 0.5f.xx) * invDstSize;
    float2 invSrcSize = GetInvTargetSize(SrcResolution);

    float4 result = 0.0f.xxxx;

    // テントフィルタ.
    result += ColorMap.SampleLevel(LinearClamp, uv + float2(-0.5f, -0.5f) * invSrcSize, 0.0f);
    result += ColorMap.SampleLevel(LinearClamp, uv + float2( 0.5f, -0.5f) * invSrcSize, 0.0f);
    result += ColorMap.SampleLevel(LinearClamp, uv + float2(-0.5f,  0.5f) * invSrcSize, 0.0f);
    result += ColorMap.SampleLevel(LinearClamp, uv + float2( 0.5f,  0.5f) * invSrcSize, 0.0f); 
    result *= 0.25f;

    // 加算合成.
    result += OutputMap[remapId];

    OutputMap[remapId] = result;
}
