//-----------------------------------------------------------------------------
// File : asdxIBLBakeSpecular.hlsl
// Desc : Specular LD Term Bake.
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
Texture2D           EnvMap      : register(t0); //!< 環境マップ.
RWTexture2D<float4> SpecularLD  : register(u0); //!< 積分結果格納先.


//-----------------------------------------------------------------------------
//      LD項の積分計算を行います.
//-----------------------------------------------------------------------------
float3 IntegrateSpecularIBL
(
    in float3       V,
    in float3       N,
    in float        roughness, // linear roughness.
    const uint      count
)
{
    float3 acc       = 0;
    float  accWeight = 0;
    float  a         = roughness * roughness;
    float  omegaP    = 4.0f * F_PI / float(EnvMapCurMipSize.x * EnvMapCurMipSize.y);

    [loop]
    for(uint i=0; i<count; ++i)
    {
        float2 eta = Hammersley(i, count);
        float3 H = SampleVndfGGX(eta, a, N);
        float3 L = 2 * dot(V, H) * H - V;

        float NdotH  = saturate(dot(N, H));
        float NdotL  = saturate(dot(N, L));
        float LdotH  = saturate(dot(L, H));
        float pdf    = D_GGX(NdotH, a) * (NdotH / (4.0f * F_PI * LdotH));
        float omegaS = 1.0f / (count * pdf);

        float  mipLevel = clamp(0.5f * log2(omegaS / omegaP), 0.0f, MipCount);
        float2 uv = ToOctahedralMapCoord(L)
;       float4 Li = EnvMap.SampleLevel(AnisotropicWrap, uv, mipLevel);

        acc += Li.rgb * NdotL;
        accWeight += NdotL;
    }

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

    float3 output;
    if (Roughness == 0.0f)
    {
        output = EnvMap.SampleLevel(AnisotropicWrap, uv, 0.0f).rgb;
    }
    else
    {
        // キューブマップのサンプリング方向を求める.
        float3 dir = FromOctahedralMapCoord(uv);

        // LD項の積分を行う.
        output = IntegrateSpecularIBL(dir, dir, Roughness, 256);
    }

    // 結果を八面体マップに書き込み.
    SpecularLD[pixelId] = float4(output, 1.0f);
}