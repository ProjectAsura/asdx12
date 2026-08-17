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
    float   Exposure;       //!< 露出値.
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
    return (luma < Threshold) ? 0.0f.xxxx : color * Exposure;
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

    const float exposure = 1.0f;
    float4 result = 0.0f.xxxx;

    // [Jimenez 2014] Jorge Jimenez, "Next Generation Post Processing in Call of Duty Advanced Warfare,
    // SIGGRAPH 2014: Advances in Real-Time Rendering in Games Course, 2014.
    // https://advances.realtimerendering.com/s2014/index.html
    // の最適化バージョン.

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

    uint  quadId  = WaveGetLaneIndex() & 0x3;;
    float offsetX = ((quadId & 1) == 0) ? 1.0f : -1.0f;
    float offsetY = ((quadId / 2) == 0) ? 1.0f : -1.0f;
 
    // A - B - C
    // |   |   |
    // D - E - F
    // |   |   |
    // G - H - I

    // 4角をフェッチ (A, C, G, I).
    float4 corner[4];
    corner[0] = SampleColor(uv + float2( offsetX,  offsetY) * invSize); // A
    corner[1] = SampleColor(uv + float2( offsetX, -offsetY) * invSize); // G
    corner[2] = SampleColor(uv + float2(-offsetX,  offsetY) * invSize); // C
    corner[3] = SampleColor(uv + float2(-offsetX, -offsetY) * invSize); // I.

    // 残りの十字方向は，Quad Intrinsicsで取得 (B, D, E, F, H)
    float4 cross[5];
    cross[0] = QuadReadAcrossX(corner[0]);          // B
    cross[1] = QuadReadAcrossY(corner[0]);          // D
    cross[2] = QuadReadAcrossDiagonal(corner[0]);   // E
    cross[3] = QuadReadAcrossX(corner[1]);          // H
    cross[4] = QuadReadAcrossY(corner[2]);          // F.

    {
        float w0 = KarisAntiFireflyWeight(corner[0].rgb, exposure);
        float w1 = KarisAntiFireflyWeight(cross[0].rgb, exposure);
        float w2 = KarisAntiFireflyWeight(cross[1].rgb, exposure);
        float w3 = KarisAntiFireflyWeight(cross[2].rgb, exposure);
 
        float4 box = ((corner[0] * w0) + (cross[0] * w1) + (cross[1] * w2) + (cross[2] * w3)) / (w0 + w1 + w2 + w3);
        result += box * 0.125f;
    }

    {
        float w0 = KarisAntiFireflyWeight(corner[1].rgb, exposure);
        float w1 = KarisAntiFireflyWeight(cross[1].rgb, exposure);
        float w2 = KarisAntiFireflyWeight(cross[2].rgb, exposure);
        float w3 = KarisAntiFireflyWeight(cross[4].rgb, exposure);
 
        float4 box = ((corner[1] * w0) + (cross[1] * w1) + (cross[2] * w2) + (cross[4] * w3)) / (w0 + w1 + w2 + w3);
        result += box * 0.125f;
    }

    {
        float w0 = KarisAntiFireflyWeight(corner[2].rgb, exposure);
        float w1 = KarisAntiFireflyWeight(cross[0].rgb, exposure);
        float w2 = KarisAntiFireflyWeight(cross[2].rgb, exposure);
        float w3 = KarisAntiFireflyWeight(cross[3].rgb, exposure);
 
        float4 box = ((corner[2] * w0) + (cross[0] * w1) + (cross[2] * w2) + (cross[3] * w3)) / (w0 + w1 + w2 + w3);
        result += box * 0.125f;
    }

    {
        float w0 = KarisAntiFireflyWeight(corner[3].rgb, exposure);
        float w1 = KarisAntiFireflyWeight(cross[2].rgb, exposure);
        float w2 = KarisAntiFireflyWeight(cross[3].rgb, exposure);
        float w3 = KarisAntiFireflyWeight(cross[4].rgb, exposure);
 
        float4 box = ((corner[3] * w0) + (cross[2] * w1) + (cross[3] * w2) + (cross[4] * w3)) / (w0 + w1 + w2 + w3);
        result += box * 0.125f;
    }

    // 結果を書き込み.
    OutputMap[remapId] = result;
}
