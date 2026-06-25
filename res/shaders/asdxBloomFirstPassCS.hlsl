//-----------------------------------------------------------------------------
// File : asdxBloomFirstPassCS.hlsl
// Desc : Compute Shader For Bloom.
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
Texture2D<float4>   ColorMap    : register(t0);
RWTexture2D<float4> OutputMap   : register(u0);


//-----------------------------------------------------------------------------
//      カラーをサンプルします.
//-----------------------------------------------------------------------------
float4 SampleColor(float2 uv)
{
    float4 color = ColorMap.SampleLevel(LinearClamp, uv, 0.0f);
    float  luma  = LuminanceBT709(color.rgb);
    return (luma < Threshold) ? 0.0f.xxxx : color;
}

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
    float2 uv = float2(remapId + 0.5f.xx) * invSize;

    float4 result = 0.0f.xxxx;

    // 中心4テクセル.
    {
        float4 c0 = SampleColor(uv + float2(-0.5f, -0.5f) * invSize);
        float4 c1 = SampleColor(uv + float2( 0.5f, -0.5f) * invSize);
        float4 c2 = SampleColor(uv + float2(-0.5f,  0.5f) * invSize);
        float4 c3 = SampleColor(uv + float2( 0.5f,  0.5f) * invSize);

        float w0 = KarisAntiFireflyWeight(c0.xyz, 1.0f);
        float w1 = KarisAntiFireflyWeight(c1.xyz, 1.0f);
        float w2 = KarisAntiFireflyWeight(c2.xyz, 1.0f);
        float w3 = KarisAntiFireflyWeight(c3.xyz, 1.0f);
 
        float4 b0 = ((c0 * w0) + (c1 * w1) + (c2 * w2) + (c3 * w3)) / (w0 + w1 + w2 + w3);
        result += b0 * 0.5f;
    }
 
    // 左上.
    {
        float4 c0 = SampleColor(uv + float2(-1.0f, -1.0f) * invSize);
        float4 c1 = SampleColor(uv + float2( 0.0f, -1.0f) * invSize);
        float4 c2 = SampleColor(uv + float2(-1.0f,  0.0f) * invSize);
        float4 c3 = SampleColor(uv + float2( 0.0f,  0.0f) * invSize);

        float w0 = KarisAntiFireflyWeight(c0.xyz, 1.0f);
        float w1 = KarisAntiFireflyWeight(c1.xyz, 1.0f);
        float w2 = KarisAntiFireflyWeight(c2.xyz, 1.0f);
        float w3 = KarisAntiFireflyWeight(c3.xyz, 1.0f);
 
        float4 b0 = ((c0 * w0) + (c1 * w1) + (c2 * w2) + (c3 * w3)) / (w0 + w1 + w2 + w3);
        result += b0 * 0.125f;
    }
 
    // 右上
    {
        float4 c0 = SampleColor(uv + float2( 0.0f, -1.0f) * invSize);
        float4 c1 = SampleColor(uv + float2( 1.0f, -1.0f) * invSize);
        float4 c2 = SampleColor(uv + float2( 0.0f,  0.0f) * invSize);
        float4 c3 = SampleColor(uv + float2( 1.0f,  0.0f) * invSize);

        float w0 = KarisAntiFireflyWeight(c0.xyz, 1.0f);
        float w1 = KarisAntiFireflyWeight(c1.xyz, 1.0f);
        float w2 = KarisAntiFireflyWeight(c2.xyz, 1.0f);
        float w3 = KarisAntiFireflyWeight(c3.xyz, 1.0f);
 
        float4 b0 = ((c0 * w0) + (c1 * w1) + (c2 * w2) + (c3 * w3)) / (w0 + w1 + w2 + w3);
        result += b0 * 0.125f;
    }

    // 左下
    {
        float4 c0 = SampleColor(uv + float2(-1.0f,  0.0f) * invSize);
        float4 c1 = SampleColor(uv + float2( 0.0f,  0.0f) * invSize);
        float4 c2 = SampleColor(uv + float2(-1.0f,  1.0f) * invSize);
        float4 c3 = SampleColor(uv + float2( 0.0f,  1.0f) * invSize);

        float w0 = KarisAntiFireflyWeight(c0.xyz, 1.0f);
        float w1 = KarisAntiFireflyWeight(c1.xyz, 1.0f);
        float w2 = KarisAntiFireflyWeight(c2.xyz, 1.0f);
        float w3 = KarisAntiFireflyWeight(c3.xyz, 1.0f);
 
        float4 b0 = ((c0 * w0) + (c1 * w1) + (c2 * w2) + (c3 * w3)) / (w0 + w1 + w2 + w3);
        result += b0 * 0.125f;
    }

    // 右下
    {
        float4 c0 = SampleColor(uv + float2( 0.0f,  0.0f) * invSize);
        float4 c1 = SampleColor(uv + float2( 1.0f,  0.0f) * invSize);
        float4 c2 = SampleColor(uv + float2( 0.0f,  1.0f) * invSize);
        float4 c3 = SampleColor(uv + float2( 1.0f,  1.0f) * invSize);

        float w0 = KarisAntiFireflyWeight(c0.xyz, 1.0f);
        float w1 = KarisAntiFireflyWeight(c1.xyz, 1.0f);
        float w2 = KarisAntiFireflyWeight(c2.xyz, 1.0f);
        float w3 = KarisAntiFireflyWeight(c3.xyz, 1.0f);
 
        float4 b0 = ((c0 * w0) + (c1 * w1) + (c2 * w2) + (c3 * w3)) / (w0 + w1 + w2 + w3);
        result += b0 * 0.125f;
    }

    // 結果を書き込み.
    OutputMap[remapId] = result;
}
