//-----------------------------------------------------------------------------
// File : asdxBRDF.hlsli
// Desc : BRDF.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#ifndef ASDX_BRDF_HLSLI
#define ASDX_BRDF_HLSLI

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxMath.hlsli"
#include "asdxTangentSpace.hlsli"
#include "asdxRandom.hlsli"

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static const float kDefaultIor = 1.5f;  // 屈折率のデフォルト値.
static const float kDefaultF0  = 0.04f; // F0のデフォルト値(ior = 1.5として計算したときの値).
static const float kDefaultAO  = 1.0f;  // アンビエントオクルージョンのデフォルト値.


//-----------------------------------------------------------------------------
//      グロシネスに変換します.
//-----------------------------------------------------------------------------
float RoughnessToGlossiness(float roughness)
{ return saturate(1.0f - roughness); }

//-----------------------------------------------------------------------------
//      ラフネスに変換します.
//-----------------------------------------------------------------------------
float GlossinessToRoughness(float glossiness)
{ return saturate(1.0f - glossiness); }

//-----------------------------------------------------------------------------
//      PBR RoughnessからTradiational Specular Powerに変換します.
//-----------------------------------------------------------------------------
float GlossinessToSpecularPower(float glossiness)
{
    // Sebastien Lagarade, "Adopting a physically based shading model", 
    // https://seblagarde.wordpress.com/2011/08/17/hello-world/
    // ※有効範囲は[2, 2048]まで.
    return exp2(10.0f * glossiness + 1.0f);
}

//-----------------------------------------------------------------------------
//      Traditional Specular Power から PBR Glossinessに変換します.
//-----------------------------------------------------------------------------
float SpecularPowerToGlossiness(float specularPower)
{ return log2(specularPower) * 0.01f - 1.0f; }

//-----------------------------------------------------------------------------
//      Traditional Specular Power から　PBR Roughnessに変換します.
//-----------------------------------------------------------------------------
float SpecularPowerToRoughness(float specularPower)
{ return SpecularPowerToRoughness(SpecularPowerToGlossiness(specularPower)); }

//-----------------------------------------------------------------------------
//      ラフネスからスペキュラー指数を求めます.
//-----------------------------------------------------------------------------
float ToSpecularPower(float roughness)
{ return (2.0f / max(0.0002f, roughness * roughness)) - 2.0f; }

//-----------------------------------------------------------------------------
//      スペキュラー指数からラフネス値を求めます.
//-----------------------------------------------------------------------------
float ToRoughness(float specularPower)
{
    // Dimiatr Lazarov, "Physically-based lighting in Call of Duty: Black Ops",
    // SIGGRAPH 2011 Cources: Advances in Real-Time Rendering in 3D Graphics.
    return sqrt(2.0f / (specularPower + 2.0f));
}

//-----------------------------------------------------------------------------
//      f0 を計算します.
//-----------------------------------------------------------------------------
float CalcF0(float ior)
{ return Pow2((1.0f - ior) / (1.0f + ior)); }

//-----------------------------------------------------------------------------
//      90度入射におけるフレネル反射率を求めます.
//-----------------------------------------------------------------------------
float CalcF90(in float3 f0)
{
    // 0.33 は 1/3の意味. 50は F_0 = 0.02~0.08の範囲を1に近づけるための調整値.
    return saturate(50.0f * dot(f0, 0.33f));
}

//-----------------------------------------------------------------------------
//      Schlickによるフレネル反射の近似値を求める.
//-----------------------------------------------------------------------------
float3 F_Schlick(in float3 f0, in float3 f90, in float u)
{ return f0 + (f90.xxx - f0) * Pow5(1.0f - u); }

//-----------------------------------------------------------------------------
//      Schlickによるフレネル反射の近似値を求める.
//-----------------------------------------------------------------------------
float F_Schlick(in float f0, in float f90, in float u)
{ return f0 + (f90 - f0) * Pow5(1.0f - u); }

//-----------------------------------------------------------------------------
//      フレネル項を計算します.
//-----------------------------------------------------------------------------
float3 F_Schlick(const float3 f0, float VoH)
{ return F_Schlick(f0, 1.0f, VoH); }

//-----------------------------------------------------------------------------
//      フレネル項を計算します.
//-----------------------------------------------------------------------------
float F_Schlick(const float f0, float VoH)
{ return F_Schlick(f0, 1.0f, VoH); }

//-----------------------------------------------------------------------------
//      フレネル項を用いて補間を行います.
//-----------------------------------------------------------------------------
float3 FresnelLerp(float3 base, float3 layer, float f0, float VoH)
{
    float fr = F_Schlick(f0, 1.0f, VoH);
    return lerp(base, layer, fr);
}

//-----------------------------------------------------------------------------
//      導体フレネルを計算します.
//-----------------------------------------------------------------------------
float3 ConductorFresnel(float3 f0, float3 bsdf, float VoH)
{ return bsdf * F_Schlick(f0, 1.0f.xxx, abs(VoH)); }

//-----------------------------------------------------------------------------
//      拡散反射率を求めます.
//-----------------------------------------------------------------------------
float3 ToKd(float3 baseColor, float metalness, float f0 = kDefaultF0)
{ return baseColor * (1.0f - f0) * (1.0f - metalness); }

//-----------------------------------------------------------------------------
//      鏡面反射率を求めます.
//-----------------------------------------------------------------------------
float3 ToKs(float3 baseColor, float metalness, float f0 = kDefaultF0)
{ return lerp(f0, baseColor, metalness); }

//-----------------------------------------------------------------------------
//      ディフューズの支配的な方向を求めます.
//-----------------------------------------------------------------------------
float3 GetDiffuseDominantDir(float3 N, float3 V, float NoV, float roughness)
{
    float a = 10.2341f * roughness - 1.51174f;
    float b = -0.511705f * roughness + 0.755868f;
    float lerpFactor = saturate((NoV * a + b) * roughness);
    return lerp(N, V, lerpFactor);
}

//-----------------------------------------------------------------------------
//      スペキュラーの支配的な方向を求めます.
//-----------------------------------------------------------------------------
float3 GetSpecularDominantDir(float3 N, float3 R, float roughness)
{
    float smoothness = saturate(1.0f - roughness);
    float lerpFactor = smoothness * (sqrt(smoothness) + roughness);
    return lerp(N, R, lerpFactor);
}

//-----------------------------------------------------------------------------
//      スペキュラーローブの半角の正接を求めます.
//-----------------------------------------------------------------------------
float GetSpecularLobeTanHalfAngle(float linearRoughness, float percentOfVolume = 0.75f)
{
    // Moving Frostbite to PBR v3.2 p.72
    float a = linearRoughness * linearRoughness;
    return a * percentOfVolume / ( 1.0 - percentOfVolume );
}

//-----------------------------------------------------------------------------
//      スペキュラーローブの半角を求めます.
//-----------------------------------------------------------------------------
float GetSpecularLobeHalfAngle(float linearRoughess, float percentOfVolume = 0.75f)
{
    // Moving Frostbite to PBR v3.2 p.72
    float tangent = GetSpecularLobeTanHalfAngle(linearRoughess, percentOfVolume);
    return atan(tangent);
}

//-----------------------------------------------------------------------------
//      スペキュラーオクルージョンを計算します.
//-----------------------------------------------------------------------------
float CalcSpecularOcclusion(float NoV, float ao, float roughness)
{
    // [Lagrade 2014] Sebastein Lagarde, Charle de Rousiers, 
    // "Moving Frostbite to Physically Based Rendering 3.0", p.77, Listining 26.
    return saturate(pow(max(NoV + ao, 0.0f), exp2(-16.0f * roughness - 1.0f)) - 1.0f + ao);
}

//-----------------------------------------------------------------------------
//      水平スペキュラーオクルージョンを計算します.
//-----------------------------------------------------------------------------
float CalcHorizonSpecularOcclusion(float RoN)
{
    // [Google 2018], "Physically Based Rendering in Filament", 
    // 5.6.19.16 Horizon specular occlusion, Listing 39.
    // https://google.github.io/filament/Filament.md.html#lighting/units/lightunitsvalidation
    
    // CalcSpecularOcclusionがパフォーマンス的に困る場合に，簡易な近似として使用する.
    float horizon = min(1.0f + RoN, 1.0f);
    return horizon * horizon;
}

//-----------------------------------------------------------------------------
//      マイクロシャドウを計算します.
//-----------------------------------------------------------------------------
float ApplyMicroShadow(float ao, float NoL, float shadow)
{
    // [Brinck 2016] Waylon Brinck, Andrew Maximov,
    // "The Technical Art of Uncharted 4", SIGGRAPH 2016, Slide 37.
    float aperture = 2.0 * ao * ao;
    float microShadow = saturate(NoL + aperture - 1.0);
    return shadow * microShadow;
}

//-----------------------------------------------------------------------------
//      ライトベイクしたオブジェクトに対するAOを計算します.
//-----------------------------------------------------------------------------
float AmbientOcclusionFresnel(float3 vertexNormalWS, float3 viewWS, float ao)
{
    // [Brinck 2016] Waylon Brinck, Andrew Maximov,
    // "The Technical Art of Uncharted 4", SIGGRAPH 2016, Slide 40.

    // ライトベイクしたオブジェクトに適用する.
    // 斜めから見たとき、 クラックの奥まで見通せないようにして、AOの暗い部分をフェードアウトする.
    float aoFadeTerm = saturate(dot(vertexNormalWS, viewWS));
    return lerp(1.0f, ao, aoFadeTerm);
}

//-----------------------------------------------------------------------------
//      SmithによるG2項を求めます.
//-----------------------------------------------------------------------------
float G2_Smith(float a2, float NoL, float NoV)
{
    // a2 = linearRoughness * linearRoughnss を渡してください.
 
    // [Lagarde 2014] Sebastien Lagarde, Charles de Rousier,
    // "Moving Frostbite to Physically Based Rendering 3.0",
    // p.12, Listing 2.
    float GGXV = NoV * sqrt(a2 + NoL * (NoL - a2 * NoL));
    float GGXL = NoL * sqrt(a2 + NoV * (NoV - a2 * NoV));

    // NOTE: "1.0f / (4.0f * NoL * NoV)"が考慮されている値なので，
    //       返却値にこれらを掛ける必要はありません.
    return 0.5f / (GGXV + GGXL);
}

//-----------------------------------------------------------------------------
//      GGXのD項を求めます.
//-----------------------------------------------------------------------------
float D_GGX(float NoH, float a2)
{
    // a2 = linearRoughness * linearRoughnss を渡してください.
    float d = (NoH * a2 - NoH) * NoH + 1.0f;
    return a2 / (F_PI * Pow2(d));
}

//-----------------------------------------------------------------------------
//      D項を計算します.
//-----------------------------------------------------------------------------
float D_Charlie(float sheenRoughness2, float NoH)
{
    // Estevez and Kulla 2017, "Production Friendly Microfacet Sheen BRDF".
    float invAlpha = 1.0f / sheenRoughness2;
    float cos2h = NoH * NoH;
    float sin2h = max(1.0f - cos2h, 0.0078125f); // 2^(-14/2), so sin2h^2 0 in fp16
    return (2.0f + invAlpha) * pow(sin2h, invAlpha * 0.5f) / F_2PI;
}

//-----------------------------------------------------------------------------
//      charlie BRDF の l項を計算します.
//-----------------------------------------------------------------------------
float L_Charlie(float x, float alpha_g)
{
    float one_minus_alpha_sq = (1.0f - alpha_g) * (1.0f - alpha_g);
    float a = lerp( 21.5473f,  25.3245f, one_minus_alpha_sq);
    float b = lerp( 3.82987f,  3.32435f, one_minus_alpha_sq);
    float c = lerp( 0.19823f,  0.16801f, one_minus_alpha_sq);
    float d = lerp(-1.97760f, -1.27393f, one_minus_alpha_sq);
    float e = lerp(-4.32054f, -4.85967f, one_minus_alpha_sq);
    return a / (1.0f + b * pow(x, c)) + d * x + e;
}

//-----------------------------------------------------------------------------
//      sheen の λ 項を計算します.
//-----------------------------------------------------------------------------
float LambdaSheen(float cos_theta, float alpha_g)
{
    return (abs(cos_theta) < 0.5f)
        ? exp(L_Charlie(cos_theta, alpha_g)) 
        : exp(2.0f * L_Charlie(0.5, alpha_g) - L_Charlie(1.0f - cos_theta, alpha_g));
}

//-----------------------------------------------------------------------------
//      V項を計算します.
//-----------------------------------------------------------------------------
float V_Charlie(float alpha_g, float NoV, float NoL)
{
    // alpha_g = sheenRoughness * sheenRoughness とします.
    float lambdaV = LambdaSheen(NoV, alpha_g);
    float lambdaL = LambdaSheen(NoL, alpha_g);
    return 1.0f / ((1.0f + lambdaV + lambdaL) * (4.0f * NoV * NoL));
}

//-----------------------------------------------------------------------------
//      V項を計算します.
//-----------------------------------------------------------------------------
float V_Neubelt(float NoV, float NoL)
{
    // Neubelt and Pettineo 2013, "Crafting a Next-gen Material Pipeline for THe Order: 1886".
    return SaturateHalf(1.0f / (4.0f * (NoL + NoV - NoL * NoV)));
}

//-----------------------------------------------------------------------------
//      光沢BRDFを計算します.
//-----------------------------------------------------------------------------
float3 SheenBRDF(Texture2D sheenLUT, SamplerState clampSampler, float3 material, float3 sheenColor, float sheenRoughness, float NoL, float NoV, float NoH)
{
    float alpha_g = sheenRoughness * sheenRoughness;
    float D = D_Charlie(alpha_g, NoH);
    float V = V_Charlie(alpha_g, NoV, NoL);
    float3 sheenBRDF = sheenColor * V * D;

    float E_NoV = sheenLUT.SampleLevel(clampSampler, float2(NoV, sheenRoughness), 0.0f).r;
    float E_NoL = sheenLUT.SampleLevel(clampSampler, float2(NoL, sheenRoughness), 0.0f).r;
 
    float  maxColor = Max3(sheenColor);
    float3 sheen_albedo_scaling = min(1.0f - maxColor * E_NoV, 1.0f - maxColor * E_NoL);
    return sheenBRDF + material * sheen_albedo_scaling;
}

//-----------------------------------------------------------------------------
//      異方性GGXのD項を計算します.
//-----------------------------------------------------------------------------
float D_GGX_Anisotropic(float NoH, float ToH, float BoH, float at, float ab)
{
    // NoV = dot(N, H)
    // ToH = dot(T, H)  ... T: Tangent.
    // BoH = dot(B, H)  ... B: Bitangent.
    // at = lerp(roughness, 1.0f, anisotropy * anisotropy)
    // ab = roughness.
    
    // [KhronosGroup 2024], KHR_materials_anisotropy,
    // https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_anisotropy
    float  a2 = at * ab;
    float3 f  = float3(ab * ToH, at * BoH, a2 * NoH);
    float  w2 = SaturateFloat(a2 / dot(f, f));
    return a2 * w2 * w2 / F_PI;
}

//-----------------------------------------------------------------------------
//      異方性GGXのV項を計算します.
//-----------------------------------------------------------------------------
float V_GGX_Anisotropic(float NoL, float NoV, float BoV, float ToV, float ToL, float BoL, float at, float ab)
{
    // [KhronosGroup 2024], KHR_materials_anisotropy,
    // https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_anisotropy
    float GGXV = NoL * length(float3(at * ToV, ab * BoV, NoV));
    float GGXL = NoV * length(float3(at * ToL, ab * BoL, NoL));
    float v = 0.5f / (GGXV + GGXL);
    return saturate(v);
}

//-----------------------------------------------------------------------------
//      異方性GGXのV項とD項を計算します.
//-----------------------------------------------------------------------------
float3 VD_GGXAnisotropic
(
    float   linearRoughness,
    float   anisotropy,
    float   NoV,
    float   NoL,
    float   NoH,
    float   ToV,
    float   ToL,
    float   ToH,
    float   BoV,
    float   BoL,
    float   BoH
)
{
    //float a = linearRoughness * linearRoughness;
    //float at = lerp(a, 1.0f, anisotropy * anisotropy);
    //float ab = a;
    float at = max(linearRoughness * (1.0f + anisotropy), 0.001f);
    float ab = max(linearRoughness * (1.0f - anisotropy), 0.001f);

    float  V = V_GGX_Anisotropic(NoL, NoV, BoV, ToV, ToL, BoL, at, ab);
    float  D = D_GGX_Anisotropic(NoH, ToH, BoH, at, ab);

    return V * D;
}

//-----------------------------------------------------------------------------
//      クリアコートBRDFを計算します.
//-----------------------------------------------------------------------------
float3 ClearCoatBRDF(float3 material, float clearCoat, float clearCoatRoughness, float NcoL, float NcoV, float NcoH, float f0 = kDefaultF0)
{
    // 引数は，下記を渡してください.
    // NcoL = dot(clearCoatNormal, L);
    // NcoV = dot(clearCoatNormal, V);
    // NcoH = dot(clearCoatNormal, H);

    // [KhronosGroup 2024] KHR_materials_clearcoat
    // https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_clearcoat.
    float  clearcoat_fresnel = f0 + (1.0f - f0) * Pow5(1.0f - abs(NcoV));
    float  clearcoat_alpha   = clearCoatRoughness * clearCoatRoughness;
    float3 clearcoat_brdf    = D_GGX(abs(NcoH), clearcoat_alpha) * G2_Smith(clearcoat_alpha, NcoL, NcoV) /(4.0f * abs(NcoV) * abs(NcoL));

    return lerp(material, clearcoat_brdf, clearCoat * clearcoat_fresnel);
}

//-----------------------------------------------------------------------------
//      クリアコートエミッシブを計算します.
//-----------------------------------------------------------------------------
float3 ClearCoatEmissive(float3 emissive, float clearCoat, float NcoV, float f0 = kDefaultF0)
{
    // クリアコート層によってエミッシブが暗くなるので、それを計算する.
    // NcoV = dot(clearCoatNormal, V);

    // [KhronosGroup 2024] KHR_materials_clearcoat
    // https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_clearcoat.
    float clearcoat_fresnel = F_Schlick(f0, 1.0f, abs(NcoV));
    return emissive * (1.0f - clearCoat * clearcoat_fresnel);
}

//-----------------------------------------------------------------------------
//      ヘアのスペキュラー項を求めます.
//-----------------------------------------------------------------------------
float ScheuermannSingleSpecularTerm(float3 T, float3 H, float exponent)
{
    // Thorsten Scheuermann, "Hair Rendering and Shading", ShaderX 3, p.244　参照.
    float ToH   = dot(T, H);
    float sinTH = sqrt(1.0f - ToH * ToH);
    return pow(sinTH, exponent);
}

//-----------------------------------------------------------------------------
//      ヘアのスペキュラー減衰を求めます.
//-----------------------------------------------------------------------------
float ScheuermannSpecularAttenuation(float NoL)
{
    // Thorsten Scheuermann, "Hair Rendering and Shading", ShaderX 3. p.246 参照.
    return saturate(1.75f * NoL + 0.25f);
}

//-----------------------------------------------------------------------------
//      Kajiya-Keyディフューズ項を評価します.
//-----------------------------------------------------------------------------
float EvaluateScheuermannDiffuse(float NoL)
{
    // Thorsten Scheuermann, "Hair Rendering and Shading", ShaderX 3, p.243 参照.
    return saturate(0.75f * NoL + 0.25f);
}

//-----------------------------------------------------------------------------
//      Scheuermannスペキュラー項を評価します.
//-----------------------------------------------------------------------------
float3 EvaluateScheuermannSpecular
(
    float3  T,              // 接線ベクトル.
    float3  N,              // 法線ベクトル.
    float3  H,              // 視線ベクトルとライトベクトルのハーフベクトル.
    float   NoL,            // 法線とライトベクトルの内積.
    float4  specularColor0, // RGB : スペキュラーカラー, A : 反射強度.
    float4  specularColor1, // RGB : スペキュラーカラー, A : 反射強度.
    float   specularShift0, // シフト量.
    float   specularShift1, // シフト量.
    float   shiftValue,     // シフトテクスチャの値.
    float   noise           // ノイズテクスチャの値.
)
{
    float3 T0 = ShiftTangent(T, N, specularShift0 + shiftValue);
    float3 T1 = ShiftTangent(T, N, specularShift1 + shiftValue);

    float3 specular0 = specularColor0.rgb * ScheuermannSingleSpecularTerm(T0, H, specularColor0.a);
    float3 specular1 = specularColor1.rgb * ScheuermannSingleSpecularTerm(T1, H, specularColor1.a);

    specular1 *= noise;

    return (specular0 + specular1) * ScheuermannSpecularAttenuation(NoL);
}

//-----------------------------------------------------------------------------
//      Kajiya-Kay BRDFを評価します.
//-----------------------------------------------------------------------------
float3 EvaluateKajiyaKay
(
    float3  T,              // 接線ベクトル.
    float3  N,              // 法線ベクトル.
    float3  V,              // 視線ベクトル.
    float3  L,              // ライトベクトル.
    float3  Kd,             // ディフューズカラー.
    float3  Ks,             // スペキュラーカラー.
    float   noise,          // ノイズテクスチャの値.
    float   primaryScale,   // プライマリーハイライト強度.
    float   secondaryWidth  // セカンダリーハイライト幅.
)
{
    // James T. Kajiya, Timothy L. Kay, "RENDERING FUR WITH THREE DIMENSIONAL TEXTURES",
    // Computer Graphics, Volume 23, Number 3, July 1989,
    // Diffuse  は Equation (14) 参照.
    // Specular は Equation (16) 参照.

    float SpecularPower0  = 80.0f * primaryScale;
    float SpecularPower1  = max(0.04f, SpecularPower0 / secondaryWidth * 4.0f);
    float Normalize0      = (SpecularPower0 + 2.0f) / (2.0f * F_PI);
    float Normalize1      = (SpecularPower1 + 2.0f) / (2.0f * F_PI);

    float cosTL = dot(T, L);
    float sinTL = ToSin(cosTL);

    float diffuse = max(sinTL, 0.0f);
    float alpha   = radians(noise * 10.0f); // チルト角(5 - 10 度)

    float cosTRL = -cosTL;
    float sinTRL =  sinTL;
    float cosTV  = dot(T, V);
    float sinTV  = ToSin(cosTV);

    // プライマリーカラーを求める.
    float cosTRL0   = cosTRL * cos(2.0f * alpha) - sinTRL * sin(2.0f * alpha);
    float sinTRL0   = ToSin(cosTRL0);
    float specular0 = max(0, cosTRL0 * cosTV + sinTRL0 * sinTV);

    // セカンダリーカラーを求める.
    float cosTRL1   = cosTRL * cos(-3.0f * alpha) - sinTRL * sin(-3.0f * alpha);
    float sinTRL1   = ToSin(cosTRL1);
    float specular1 = max(0, cosTRL1 * cosTV + sinTRL1 * sinTV);

    // スペキュラー値.
    float power0 = pow(specular0, SpecularPower0) * Normalize0;
    float power1 = pow(specular1, SpecularPower1) * Normalize1;

    // レンダリング方程式の余弦項.
    float NoL = saturate(dot(N, L));

    // BRDFを評価.
    float3 fd = Kd * diffuse / F_PI;
    float3 fs = SaturateHalf(Ks * (power0 + power1) * 0.5f);  // 2灯焚いているので2で割る(=0.5を掛ける).

    return (fd + fs) * NoL;
}

//-----------------------------------------------------------------------------
//      計算により眼球用の高さを求めます.
//-----------------------------------------------------------------------------
float ProcedualHeightForEye(float radius, float anteriorChamberDepth)
{
    // Jorge Jimenez, Javier von der Pahlen,
    // "Next-Generation Character Rendering", GDC 2013
    // Eye Rendering セクション参照.
    //const float anteriorChamberDepth = 3.23f; // 3.23[nm] from [Lackner 2005]
    return anteriorChamberDepth * saturate(1.0f - 18.4f * radius * radius);
}

//-----------------------------------------------------------------------------
//      屈折ベクトルを計算します.
//-----------------------------------------------------------------------------
float3 CalcRefraction(float n1, float n2, float3 N, float3 V)
{
    float NdotV = dot(N, V);
    float n = (NdotV > 0.0f) ? (n1 / n2) : (n2 / n1); 

    // "Real-time Rendering Third Edition", 9.5 Refraction, p.396
    // 式(9.31), 式(9.32)参照.
    // 相対屈折率 n = (媒質1の屈折率 n1 / 媒質2の屈折率 n2).
    float w = n * NdotV;
    float k = sqrt(1.0f + (w - n) * (w + n));
    return (w - k) * N - n * V;
}

//-----------------------------------------------------------------------------
//      視差マッピングによる屈折後のテクスチャ座標を計算します.
//-----------------------------------------------------------------------------
float2 ParallaxRefraction
(
    float2      texcoord,               // テクスチャ座標.
    float       height,                 // 高さ.
    float       parallaxScale,          // 視差スケール.
    float3      viewW,                  // ワールド空間の視線ベクトル.
    float3x3    world                   // ワールド行列.
)
{
    // Jorge Jimenez, Javier von der Pahlen,
    // "Next-Generation Character Rendering", GDC 2013
    // Eye Rendering セクション参照.
    float2 viewL = mul(viewW, (float3x2)world);
    float2 offset = height * viewL;
    offset.y = -offset.y;
    return texcoord - parallaxScale * offset;
}

//-----------------------------------------------------------------------------
//      物理ベースによる屈折後のテクスチャ座標を計算します.
//-----------------------------------------------------------------------------
float2 PhysicallyBasedRefraction
(
    float2      texcoord,           // テクスチャ座標.
    float       height,             // 高さ.
    float       mask,               // 網膜から強膜への補間値.
    float       n1,                 // 媒質1の屈折率.
    float       n2,                 // 媒質2の屈折率.
    float3      normalW,            // 法線ベクトル.
    float3      viewW,              // ワールド空間での視線ベクトル.
    float3      frontNormalW,       // 眼球の視線ベクトル
    float3x3    world               // ワールド行列
)
{
    // 屈折ベクトルを求める.
    float3 refractedW = CalcRefraction(n1, n2, normalW, viewW);

    // Jorge Jimenez, Javier von der Pahlen,
    // "Next-Generation Character Rendering", GDC 2013
    // Eye Rendering セクション参照.
    float  cosAlpha = dot(frontNormalW, -refractedW);
    float  dist     = height / cosAlpha;
    float3 offsetW  = dist * refractedW;

    // ローカルに変換
    float2 offsetL = mul(offsetW, world).xy;

    return texcoord + float2(mask, -mask) * offsetL;
}

//-----------------------------------------------------------------------------
//      Lambert BRDFの形状にもとづくサンプリングを行います.
//-----------------------------------------------------------------------------
float3 SampleLambert(float2 u)
{
    float z = 1.0f - 2.0f * u.x;
    float r = sqrt(max(0.0f, 1.0f - z * z));
    float phi = 2.0f * F_PI * u.y;

    // PDF = 1.0f / (4.0f * F_PI);
    return float3(r * cos(phi), r * sin(phi), z);
}

//-----------------------------------------------------------------------------
//      Phong BRDFの形状にもとづくサンプリングを行います.
//-----------------------------------------------------------------------------
float3 SamplePhong(float2 u, float shininess)
{
    float phi = 2.0f * F_PI * u.x;
    float cosTheta = pow(1.0f - u.y, 1.0f / (shininess + 1.0f));
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

    // PDF = normalizationTerm * pow(cosTheta, shininess)
    //     = ((1.0f + shininess) / (2.0f * F_PI)) * pow(cosTheta, shininess)
    return float3(
        sinTheta * cos(phi),
        sinTheta * sin(phi),
        cosTheta);
}

//-----------------------------------------------------------------------------
//      GGX BRDFの形状にもとづくサンプリングを行います.
//-----------------------------------------------------------------------------
float3 SampleGGX(float2 u, float a)
{
    // a = linearRoughness * linearRoughness とします.
    float phi = 2.0 * F_PI * u.x;
    float cosTheta = sqrt( (1.0 - u.y) / (u.y * (a * a - 1.0) + 1.0) );
    float sinTheta = sqrt( 1.0 - cosTheta * cosTheta );

    // PDF = D * NdotH;
    // Jacobian  = 1.0f / (4.0f * VdotH);
    // Final PDF = D * NdotH / (4.0f * VdotH).
    return float3(
        sinTheta * cos(phi),
        sinTheta * sin(phi),
        cosTheta);
}

//-----------------------------------------------------------------------------
//      可視法線分布を考慮したGGX BRDFの形状にもとづくサンプリングを行います.
//-----------------------------------------------------------------------------
float3 SampleVndfGGX(float2 u, float2 a, float3 wi)
{
    // a = linearRoughness * linearRoughness とします.
    float3 V = normalize(float3(wi.xy * a, wi.z));

    // [Dupuy 2023] Jonathan Dupuy, Anis Benyoub,
    // "Sampling Visible GGX Normals with Spherical Caps",
    // High-Performance Graphics 2023.
    float phi = 2.0f * F_PI * u.x;
    float z = (1.0f - u.y) * (1.0f + V.z) - V.z;
    float sinTheta = sqrt(saturate(1.0f - z * z));
    float x = sinTheta * cos(phi);
    float y = sinTheta * sin(phi);
    float3 H = float3(x, y, z) + V;

    H = normalize(float3(H.xy * a, H.z));

    // Final PDF = G1(N, H) * D(N) / (4.0f * NoH);
    // cf. [Dupuy 2023] Equation (20).
    return H;
}

//-----------------------------------------------------------------------------
//      バランスヒューリスティック.
//-----------------------------------------------------------------------------
float BalanceHeuristic(float nf, float pf, float ng, float pg)
{ return (nf * pf) / (nf * pf + ng * pg); }

//-----------------------------------------------------------------------------
//      パワーヒューリスティック.
//-----------------------------------------------------------------------------
float PowerHeuristic(float nf, float pf, float ng, float pg)
{ 
    float f = nf * pf;
    float g = ng * pg;
    return (f * f) / (f * f + g * g);
}

//-----------------------------------------------------------------------------
//        Toksvigフィルタを適用します.
//-----------------------------------------------------------------------------
float ToksvigRoughness(float3 normal, float roughness)
{
    float length_normal = length(normal);
    float shininess = 1.0f - roughness;
    float toksvig_shininess = length_normal * shininess / (length_normal + shininess * (1.0f - length_normal));
    return 1.0f - toksvig_shininess;
}

//-----------------------------------------------------------------------------
//      Tokuyoshi-Kaplanyanフィルタを適用します.
//-----------------------------------------------------------------------------
float TokuyoshiRoughness(float3 normal, float roughness, float sigma2, float kappa)
{
    // Yusuke Tokuyoshi and Anton S. Kaplanyan, "Improved Geometric Specular Antialiasing",
    // ACM SIGGRAPH Symposium on Interactive 3D Graphics and Games 2019, 
    // sigma2 : screen-space variance.
    // kappa  : clamping threshold.
    // ※ sigma^2 = 0.25, kappa = 0.18 in the paper.
    float3 dndu = ddx(normal);
    float3 dndv = ddy(normal);
    float variance = sigma2 * (dot(dndu, dndu) + dot(dndv, dndv));
    float kernelRoughness2 = min(2.0f * variance, kappa);
    return sqrt(saturate(roughness * roughness + kernelRoughness2));
}

//-----------------------------------------------------------------------------
//      線形ラフネスからミップレベルを求めます.
//-----------------------------------------------------------------------------
float RoughnessToMipLevel(float linearRoughness, float mipCount)
{
    return (mipCount - 1) * linearRoughness;
}

//-----------------------------------------------------------------------------
//      疑似油膜表現.
//-----------------------------------------------------------------------------
float3 FakeFilm(float3 V, float3 N, float mask, float thickness, float ior)
{
    // 高木康行, "モンスターハンター：ワールド アーティストによるシェーダ作成", CEDEC 2018.
    float cos0 = abs(dot(V, N));

    cos0 *= mask;
    float tr = cos0 * thickness - ior;
    float3 n_color = (cos((tr * 35.0f) * float3(0.71f, 0.87f, 1.0f)) * -0.5f) + 0.5f;
    n_color = lerp(n_color, float3(0.5f, 0.5f, 0.5f), tr);
    n_color *= n_color * 2.0f;
    return n_color;
}

//-----------------------------------------------------------------------------
//      距離に基づいてラフネスを計算します.
//-----------------------------------------------------------------------------
float ComputeDistanceBaseRoughness
(
    float distIntersectToShadePoint,
    float distIntersectionToProbeCenter,
    float linearRoughness
)
{
    // [Lagarde 2014] Sebastien Lagarde, Charles de Rousier,
    // "Moving Frostbite to Physically Based Rendering 3.0",
    // p.72, Listing 25.
    
    // To avoid artifacts we clamp to the original linearRoughness
    // which introduces an acceptable bias and allows conversion
    // of mirror reflection behavor for a smooth surfaces.
    float newLinearRoughness = clamp(distIntersectToShadePoint / distIntersectionToProbeCenter * linearRoughness, 0.0f, linearRoughness);
    return lerp(newLinearRoughness, linearRoughness, linearRoughness);
}

//-----------------------------------------------------------------------------
//      アンビエントオクルージョンを取得します.
//-----------------------------------------------------------------------------
float GetAO(float bakedAO, float postEffectAO)
{
    // [Lagarde 2014] Sebastien Lagarde, Charles de Rousier,
    // "Moving Frostbite to Physically Based Rendering 3.0", p.80.
    return min(bakedAO, postEffectAO);
}

//-----------------------------------------------------------------------------
//      分散後の屈折率を取得します.
//-----------------------------------------------------------------------------
float3 CalcDispersionIOR(float ior, float dispersion)
{
    // [Khronos 2024] KHR_materials_dispersion.
    // https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_dispersion
    float halfSpread = (ior - 1.0f) * 0.025f * dispersion;
    return float3(ior - halfSpread, ior, ior + halfSpread);
}

#endif//ADX_BRDF_HLSLI
