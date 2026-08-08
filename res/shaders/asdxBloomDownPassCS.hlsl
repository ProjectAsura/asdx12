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
    uint2 dstSize = GetTargetSize(DstResolution);

    if (any(remapId >= dstSize))
        return;

    float2 uv = float2(remapId + 0.5f.xx) / float2(dstSize);

    float2 invSrcSize = GetInvTargetSize(SrcResolution);

    float4 result = 0.0f.xxxx;
 
    // [Jimenez 2014] Jorge Jimenez, "Next Generation Post Processing in Call of Duty Advanced Warfare,
    // SIGGRAPH 2014: Advances in Real-Time Rendering in Games Course, 2014.
    // https://advances.realtimerendering.com/s2014/index.html
    // の最適化バージョン.

    // 中心4テクセル.
    {
        float4 c0 = ColorMap.SampleLevel(LinearClamp, uv + float2(-0.5f, -0.5f) * invSrcSize, 0.0f);
        float4 c1 = ColorMap.SampleLevel(LinearClamp, uv + float2( 0.5f, -0.5f) * invSrcSize, 0.0f);
        float4 c2 = ColorMap.SampleLevel(LinearClamp, uv + float2(-0.5f,  0.5f) * invSrcSize, 0.0f);
        float4 c3 = ColorMap.SampleLevel(LinearClamp, uv + float2( 0.5f,  0.5f) * invSrcSize, 0.0f);

        result += (c0 + c1 + c2 + c3) * 0.125; // = 0.25 * 0.5f = 0.125f.
    }
 
    // QuadID を求める. 
    // 参考: https://microsoft.github.io/DirectX-Specs/d3d/HLSL_SM_6_6_Derivatives.html
    uint  quadId  = WaveGetLaneIndex() & 0x3;
    float offsetX = ((quadId & 1) == 0) ? 1.0f : -1.0f;
    float offsetY = ((quadId / 2) == 0) ? 1.0f : -1.0f;
 
    // A - B - C
    // |   |   |
    // D - E - F
    // |   |   |
    // G - H - I

    // 4角を取得 (A, C, G, I).
    float4 corner[4];
    corner[0] = ColorMap.SampleLevel(LinearClamp, uv + float2( offsetX,  offsetY) * invSrcSize, 0.0f); // A
    corner[1] = ColorMap.SampleLevel(LinearClamp, uv + float2( offsetX, -offsetY) * invSrcSize, 0.0f); // G
    corner[2] = ColorMap.SampleLevel(LinearClamp, uv + float2(-offsetX,  offsetY) * invSrcSize, 0.0f); // C
    corner[3] = ColorMap.SampleLevel(LinearClamp, uv + float2(-offsetX, -offsetY) * invSrcSize, 0.0f); // I.

    // 残りの十字方向は，Quad Intrinsicsで取得 (B, D, E, F, H)
    float4 cross[5];
    cross[0] = QuadReadAcrossX(corner[0]);          // B
    cross[1] = QuadReadAcrossY(corner[0]);          // D
    cross[2] = QuadReadAcrossDiagonal(corner[0]);   // E
    cross[3] = QuadReadAcrossX(corner[1]);          // H
    cross[4] = QuadReadAcrossY(corner[2]);          // F.

    // 2x2のボックスフィルタに 0.125 の重みづけ.
    result += (corner[0] + cross[0] + cross[1] + cross[2]) * 0.03125f; // = 0.25 * 0.125f = 0.03125f.
    result += (corner[1] + cross[1] + cross[2] + cross[4]) * 0.03125f;
    result += (corner[2] + cross[0] + cross[2] + cross[3]) * 0.03125f;
    result += (corner[3] + cross[2] + cross[3] + cross[4]) * 0.03125f;

    // 結果を書き込み.
    OutputMap[remapId] = result;
}
