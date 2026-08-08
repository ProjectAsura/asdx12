//-----------------------------------------------------------------------------
// File : asdxBloomFinalPass.hlsl
// Desc : Compute Shader For Bloom Final Pass.
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
    uint    SrcResolution;  //!< 入力解像度(ブルームデータ).
    uint    DstResolution;  //!< 出力解像度.
    uint2   Reserved;       //!< 予約領域.
};

//-----------------------------------------------------------------------------
// Resources
//-----------------------------------------------------------------------------
Texture2D<float4>   BloomMap : register(t0);    //!< 合成済みブルームデータ.
Texture2D<float4>   SrcMap   : register(t1);    //!< ブルーム入力データ.
RWTexture2D<float4> DstMap   : register(u0);    //!< 出力データ.


//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void main(uint3 disptachId : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
    uint2 remapId = RemapLane8x8(disptachId.xy, groupIndex);
    uint2 dstSize = GetTargetSize(DstResolution);
    
    if (any(remapId >= dstSize))
        return;

    float2 uv = (float2(remapId) + 0.5f.xx) / float2(dstSize);
    float2 invSrcSize = GetInvTargetSize(SrcResolution);

    float4 result = 0.0f.x;

    // テントフィルタ.
    result += BloomMap.SampleLevel(LinearClamp, uv + float2(-0.5f, -0.5f) * invSrcSize, 0.0f);
    result += BloomMap.SampleLevel(LinearClamp, uv + float2( 0.5f, -0.5f) * invSrcSize, 0.0f);
    result += BloomMap.SampleLevel(LinearClamp, uv + float2(-0.5f,  0.5f) * invSrcSize, 0.0f);
    result += BloomMap.SampleLevel(LinearClamp, uv + float2( 0.5f,  0.5f) * invSrcSize, 0.0f); 
    result *= 0.25f;

    // 元データを加味.
    result += SrcMap.SampleLevel(LinearClamp, uv, 0.0f);

    // 結果を格納.
    DstMap[remapId] = result;
}
