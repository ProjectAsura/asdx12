//-----------------------------------------------------------------------------
// File : asdxSkySphereCS.hlsl
// Desc : Compute Shader for Sky Sphere.
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
    float4x4 View;  //!< ビュー行列.
    float4x4 Proj;  //!< 射影行列.
};

///////////////////////////////////////////////////////////////////////////////
// Constants structure
///////////////////////////////////////////////////////////////////////////////
cbuffer Constants : register(b1)
{
    uint2    Size;      //!< レンダーターゲットサイズ.
    float2   InvSize;   //!< レンダーターゲットサイズの逆数.
};

//-----------------------------------------------------------------------------
// Resources.
//-----------------------------------------------------------------------------
Texture2D           SphereMap    : register(t0);
RWTexture2D<float4> RenderTarget : register(u0);


//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
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

    // スフィアマップのテクスチャ座標に変換.
    float2 uv = ToSphereMapCoord(rayDir);
 
    // スフィアマップをフェッチ.
    float4 color = SphereMap.SampleLevel(LinearClamp, uv, 0.0f);

    // 結果を書き込む.
    RenderTarget[pixelId] = color;
}
