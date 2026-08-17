//-----------------------------------------------------------------------------
// File : asdxStarFirstPassCS.hlsl
// Desc : Compute Shader For Star.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxComputeUtil.hlsli"
#include "asdxColor.hlsli"
#include "asdxSamplers.hlsli"


///////////////////////////////////////////////////////////////////////////////
// CbParam constant buffers.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbParam : register(b0)
{
    uint    SrcResolution;  //!< 入力解像度.
    uint    DstResolution;  //!< 出力解像度.
    float   Threshold;      //!< ブルーム閾値.
    uint    Reserved;       //!< 予約領域.
};

//-----------------------------------------------------------------------------
// Resources
//-----------------------------------------------------------------------------
Texture2D<float4>   ColorMap  : register(t0);
RWTexture2D<float4> OutputMap : register(u0);


//-----------------------------------------------------------------------------
//      カラーをサンプルします.
//-----------------------------------------------------------------------------
float4 SampleColor(float2 uv)
{
    float4 color = ColorMap.SampleLevel(LinearClamp, uv, 0.0f);
    float luma = LuminanceBT709(color.rgb);
    return (luma < Threshold) ? 0.0f.xxxx : color;
}

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

    float2 invSize = 1.0f.xx / float2(dstSize);
    float2 uv = float2(remapId + 0.5f.xx) * invSize;

    // 結果を書き込み.
    OutputMap[remapId] = SampleColor(uv);
}
