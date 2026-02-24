//-----------------------------------------------------------------------------
// File : asdxBackgroundCubeCS.hlsl
// Desc : Background Renderer with Cube Map.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxMath.hlsli"
#include "asdxComputeUtil.hlsli"
#include "asdxSamplers.hlsli"
#include "asdxTextureUtil.hlsli"


///////////////////////////////////////////////////////////////////////////////
// Param structure
///////////////////////////////////////////////////////////////////////////////
cbuffer Param : register(b0)
{
    float4x4 View;
    float4x4 Proj;
    uint     Size;
    float2   InvSize;
};

//-----------------------------------------------------------------------------
// Resources.
//-----------------------------------------------------------------------------
TextureCube         CubeMap      : register(t0);
RWTexture2D<float4> RenderTarget : register(u0);

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void main( uint3 dispatchId : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex )
{
    // ピクセル番号を取得.
    uint2 pixelId = RemapLane8x8(dispatchId.xy, groupIndex);
    if (any(pixelId >= Size))
        return;
 
    // [-1, 1] に変換. y方向はDirectXなので反転させる.
    float2 pixel = (float2(pixelId) * InvSize);
    pixel.y = 1.0f - pixel.y;
    pixel = pixel * 2.0f - 1.0f;

    // カメラからスクリーンへのレイを求める.
    float3 rayDir = CalcRayDir(pixel, View, Proj);

    // キューブマップをフェッチ.
    float4 color = CubeMap.SampleLevel(LinearClamp, rayDir, 0.0f);

    // 結果を書き込む.
    RenderTarget[pixelId] = color;
}
