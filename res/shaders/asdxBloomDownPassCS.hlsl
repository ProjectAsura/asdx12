//----------------------------------------------------------------------------
// File : asdxBloomDownPassCS.hlsl
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
    uint2 dstSize;
    dstSize.x = DstResolution & 0xFFFF;
    dstSize.y = (DstResolution >> 16) & 0xFFFF;

    if (any(remapId >= dstSize))
        return;

    float2 invSize = 1.0f.xx / float2(dstSize);
    float2 uv = float2(remapId + 0.5f.xx) / float2(dstSize);

    float4 result = 0.0f.xxxx;
 
    // 中心4テクセル.
    {
        float4 c0 = ColorMap.SampleLevel(LinearClamp, uv + float2(-0.5f, -0.5f) * invSize, 0.0f);
        float4 c1 = ColorMap.SampleLevel(LinearClamp, uv + float2( 0.5f, -0.5f) * invSize, 0.0f);
        float4 c2 = ColorMap.SampleLevel(LinearClamp, uv + float2(-0.5f,  0.5f) * invSize, 0.0f);
        float4 c3 = ColorMap.SampleLevel(LinearClamp, uv + float2( 0.5f,  0.5f) * invSize, 0.0f);

        float4 b0 = (c0 + c1 + c2 + c3) * 0.25f;
        result += b0 * 0.5f;
    }
 
    // 左上.
    {
        float4 c0 = ColorMap.SampleLevel(LinearClamp, uv + float2(-1.0f, -1.0f) * invSize, 0.0f);
        float4 c1 = ColorMap.SampleLevel(LinearClamp, uv + float2( 0.0f, -1.0f) * invSize, 0.0f);
        float4 c2 = ColorMap.SampleLevel(LinearClamp, uv + float2(-1.0f,  0.0f) * invSize, 0.0f);
        float4 c3 = ColorMap.SampleLevel(LinearClamp, uv + float2( 0.0f,  0.0f) * invSize, 0.0f);

        float4 b0 = (c0 + c1 + c2 + c3) * 0.25f;
        result += b0 * 0.125f;
    }
 
    // 右上
    {
        float4 c0 = ColorMap.SampleLevel(LinearClamp, uv + float2( 0.0f, -1.0f) * invSize, 0.0f);
        float4 c1 = ColorMap.SampleLevel(LinearClamp, uv + float2( 1.0f, -1.0f) * invSize, 0.0f);
        float4 c2 = ColorMap.SampleLevel(LinearClamp, uv + float2( 0.0f,  0.0f) * invSize, 0.0f);
        float4 c3 = ColorMap.SampleLevel(LinearClamp, uv + float2( 1.0f,  0.0f) * invSize, 0.0f);

        float4 b0 = (c0 + c1 + c2 + c3) * 0.25f;
        result += b0 * 0.125f;
    }

    // 左下
    {
        float4 c0 = ColorMap.SampleLevel(LinearClamp, uv + float2(-1.0f,  0.0f) * invSize, 0.0f);
        float4 c1 = ColorMap.SampleLevel(LinearClamp, uv + float2( 0.0f,  0.0f) * invSize, 0.0f);
        float4 c2 = ColorMap.SampleLevel(LinearClamp, uv + float2(-1.0f,  1.0f) * invSize, 0.0f);
        float4 c3 = ColorMap.SampleLevel(LinearClamp, uv + float2( 0.0f,  1.0f) * invSize, 0.0f);

        float4 b0 = (c0 + c1 + c2 + c3) * 0.25f;
        result += b0 * 0.125f;
    }

    // 右下
    {
        float4 c0 = ColorMap.SampleLevel(LinearClamp, uv + float2( 0.0f,  0.0f) * invSize, 0.0f);
        float4 c1 = ColorMap.SampleLevel(LinearClamp, uv + float2( 1.0f,  0.0f) * invSize, 0.0f);
        float4 c2 = ColorMap.SampleLevel(LinearClamp, uv + float2( 0.0f,  1.0f) * invSize, 0.0f);
        float4 c3 = ColorMap.SampleLevel(LinearClamp, uv + float2( 1.0f,  1.0f) * invSize, 0.0f);

        float4 b0 = (c0 + c1 + c2 + c3) * 0.25f;
        result += b0 * 0.125f;
    }

    // 結果を書き込み.
    OutputMap[remapId] = result;
}
