//-----------------------------------------------------------------------------
// File : asdxIBLBakeDFG.hlsl
// Desc : DFG Term Bake.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxRandom.hlsli"
#include "asdxBRDF.hlsli"
#include "asdxTangentSpace.hlsli"
#include "asdxTextureUtil.hlsli"
#include "asdxSamplers.hlsli"
#include "asdxComputeUtil.hlsli"


///////////////////////////////////////////////////////////////////////////////
// BakeParam constant buffers.
///////////////////////////////////////////////////////////////////////////////
cbuffer BakeParam : register(b0)
{
    uint2 TargetSize;   //!< レンダーターゲットサイズ.
    float Roughness;    //!< 線形ラフネス.
    float MipCount;     //!< ミップマップ数.
};

///////////////////////////////////////////////////////////////////////////////
// EnvMapParam constant buffers.
///////////////////////////////////////////////////////////////////////////////
cbuffer EnvMapParam : register(b1)
{
    uint2 EnvMapMip0Size;
    uint2 EnvMapCurMipSize;
}

//-----------------------------------------------------------------------------
// Resources.
//-----------------------------------------------------------------------------
Texture2D           EnvMap     : register(t0); //!< 環境マップ.
RWTexture2D<float4> DiffuseLD  : register(u0); //!< 積分結果格納先.


//-----------------------------------------------------------------------------
//      ディフューズのLD項を積分します.
//-----------------------------------------------------------------------------
float3 IntegrateDiffuseIBL(in float3 N, const uint count)
{
    float3 acc       = 0.0f;
    float  accWeight = 0.0f;

    float omegaP = (4.0f * F_PI) / float(EnvMapCurMipSize.x * EnvMapCurMipSize.y);
    float bias   = 0.0f;
 
    // 接線空間を求める.
    float3 T, B;
    CalcONB(N, T, B);

    [loop]
    for(uint i=0; i<count; ++i)
    {
        // 超一様分布列を取得.
        float2 u = Hammersley(i, count);

        // BRDFにもとづく重点サンプリング.
        float3 L = SampleLambert(u);
        L = FromTangentSpaceToWorld(L, T, B, N);

        float NdotL = saturate(dot(N, L));
        if (NdotL > 0.0f)
        {
            // ミップマップフィルタ重点サンプリング.
            float pdf      = NdotL / F_PI;
            float omegaS   = 1.0f / max(count * pdf, 1e-8f);
            float mipLevel = clamp(0.5f * (log2(omegaS) - log2(omegaP)) + bias, 0.0f, MipCount);

            float2 uv = ToOctahedralMapCoord(L);
            acc += EnvMap.SampleLevel(AnisotropicWrap, uv, mipLevel).rgb;
            accWeight += 1.0f;
        }
    }

    if (accWeight == 0.0f)
    { return acc; }

    return acc / accWeight;
}

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
    // モートンコードでピクセル番号を並び替え.
    uint2 pixelId = RemapLane8x8(dispatchId.xy, groupIndex);
    if (any(pixelId >= TargetSize))
        return; // はみ出たら即終了.

    // テクスチャ座標算出.
    float2 uv = float2(pixelId + 0.5f) / float2(TargetSize);
    uv.y = 1.0f - uv.y;

    // キューブマップのサンプリング方向を求める.
    float3 dir = FromOctahedralMapCoord(uv);

    // LD項の積分を行う.
    float3 output = IntegrateDiffuseIBL(dir, 256);

    // 結果を八面体マップに書き込み.
    DiffuseLD[pixelId] = float4(output, 1.0f);
}