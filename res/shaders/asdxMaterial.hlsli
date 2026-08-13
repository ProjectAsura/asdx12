//-----------------------------------------------------------------------------
// File : asdxMaterial.hlsli
// Desc : Material.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#ifndef ASDX_MATERIAL_HLSLI
#define ASDX_MATERIAL_HLSLI

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxMath.hlsli"
#include "asdxBRDF.hlsli"


///////////////////////////////////////////////////////////////////////////////
// Material structure
///////////////////////////////////////////////////////////////////////////////
struct Material
{
    float3  BaseColor;              //!< ベースカラー.
    float   Metalness;              //!< メタルネス.
    float   LinearRoughness;        //!< 線形ラフネス.
    float   Ior;                    //!< 屈折率.
};

///////////////////////////////////////////////////////////////////////////////
// LightingDots structure
///////////////////////////////////////////////////////////////////////////////
struct LightingDots
{
    float NoL;  //!< 法線とライトベクトルの内積.
    float NoV;  //!< 法線とビューベクトルの内積.
    float NoH;  //!< 法線とハーフベクトルの内積.
    float VoH;  //!< ビューベクトルとハーフベクトルの内積.
};

//-----------------------------------------------------------------------------
//      ライティング用の内積を計算します.
//-----------------------------------------------------------------------------
LightingDots CalcDots(float3 T, float3 B, float3 N, float3 V, float3 L, float3 Nc)
{
    // ここは純粋な内積計算のみです.
    // abs()やsaturate()は使う側で行ってください.

    float3 H = normalize(V + L);
 
    LightingDots result = (LightingDots)0;
    result.NoL = dot(N, L);
    result.NoV = dot(N, V);
    result.NoH = dot(N, H);
    result.VoH = dot(V, H);

    return result;
}

//-----------------------------------------------------------------------------
//      直接光を評価します.
//-----------------------------------------------------------------------------
float3 EvaluateDirectLight
(
    Material        material,
    LightingDots    dots
)
{
    float  f0 = CalcF0(material.Ior);
    float3 Kd = ToKd(material.BaseColor, material.Metalness, f0);
    float3 Ks = ToKs(material.BaseColor, material.Metalness, f0);

    float3 diffuse = Kd / F_PI;

    float a2  = material.LinearRoughness * material.LinearRoughness;
    float f90 = CalcF90(Ks);

    float  D = D_GGX(dots.NoH, a2);
    float  V = G2_Smith(a2, abs(dots.NoL), abs(dots.NoV));
    float3 F = F_Schlick(Ks, f90.xxx, abs(dots.VoH));
    float3 specular = D * V * F;

    float3 result = diffuse + specular;

    return result * abs(dots.NoL);
}

//-----------------------------------------------------------------------------
//      IBLを評価します.
//-----------------------------------------------------------------------------
float3 EvaluateIBL
(
    TextureCube     DiffuseLD,      // Diffuse LD キューブマップ.
    TextureCube     SpecularLD,     // Specular LD キューブマップ.
    Texture2D       DFGMap,         // DFG テクスチャ.
    SamplerState    wrapSampler,    // 繰り返しサンプラー.
    SamplerState    clampSampler,   // クランプサンプラー.
    Material        material,       // マテリアルデータ.
    float3          N,              // 法線ベクトル.
    float3          V               // ビューベクトル.
)
{
    float a = material.LinearRoughness * material.LinearRoughness;

    float3 R = reflect(-V, N);
    float3 dominantR = GetSpecularDominantDir(N, R, a);

    float2 mapSize;
    float  mipLevels;
    SpecularLD.GetDimensions(0, mapSize.x, mapSize.y, mipLevels);
    float textureSize = max(mapSize.x, mapSize.y);

    // 放射輝度取得.
    // L * D * (f0 * Gvis * (1 - Fc) + Gvis * Fc) * cosTheta / (4 * NdotL * NdotV).
    float  mipLevel = RoughnessToMipLevel(material.LinearRoughness, mipLevels);
    float3 radiance = SpecularLD.SampleLevel(wrapSampler, R, mipLevel).xyz;

    float NoV = abs(dot(N, V));

    // 事前積分したDFGをサンプルする.
    // Fc = ( 1 - HdotL )^5
    // PreIntegratedDFG.r = Gvis * (1 - Fc)
    // PreIntegratedDFG.g = Gvis * Fc
    float2 uv = float2(max(NoV, 0.5f / textureSize), 1.0f - material.LinearRoughness);   // V方向はDirectXなので反転させている.
    float2 preDFG = DFGMap.SampleLevel(clampSampler, uv, 0).xy;

    float  f0 = CalcF0(material.Ior);
    float3 F0 = lerp(f0, material.BaseColor, material.Metalness);

    // ラフネスに依存したフレネル項. [Fdez-Agüera 2019]
    float3 Fr = max((1.0f - material.LinearRoughness).xxx, F0) - F0;
    float3 Ks = F0 + Fr * Pow5(1.0f - NoV);

    // 単一散乱.
    float3 FssEss = Ks * preDFG.x + preDFG.y;
    float3 diffuseColor = material.BaseColor * (1.0f - f0) * (1.0f - material.Metalness);

    // 多重散乱.
    // [Fdez-Agüera 2019] Carmelo J. Fdez-Agüera,
    // "A Multiple-Scattering Microfacet Model for Real-Time Image-based Lighting",
    // Journal of Compute Graphics Techniques, Vol.8, No.1, 2019.
    float  Ems    = 1.0f - (preDFG.x + preDFG.y);
    float3 F_avg  = F0 + (1.0f - F0) / 21.0f;
    float3 FmsEms = Ems * FssEss * F_avg / (1.0f - F_avg * Ems);
    float3 Kd     = diffuseColor * (1.0f - FssEss - FmsEms);

    // 放射照度取得.
    float3 irradiance = DiffuseLD.Sample(wrapSampler, N).rgb;

    // 計算結果を返却.
    return FssEss * radiance + (FmsEms + Kd) * irradiance;
}
 
#endif//ASDX_MATERIAL_HLSLI
