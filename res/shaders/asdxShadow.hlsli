//-----------------------------------------------------------------------------
// File : asdxShadow.hlsli
// Desc : Shadowing Utility.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#ifndef ASDX_SHADOW_HLSLI
#define ASDX_SHADOW_HLSLI


#define ASDX_SHADOW_EPSILON         (1.192092896e-07F)  // イプシロン.
#define ASDX_SHADOW_MANTISSA_BITS   (23)                // 仮数部.

typedef SamplerComparisonState ShadowSampler;

#ifdef ASDX_COMPUTE_SHADER
//-----------------------------------------------------------------------------
//      シャドウマップをサンプルします(コンピュートシェーダ用).
//-----------------------------------------------------------------------------
float4 SampleShadow
(
    Texture2DArray  shadowMap,      // カスケードシャドウマップ.
    ShadowSampler   shadowSmp,      // シャドウマップサンプラー.
    float2          coord,          // シャドウマップ座標.
    uint            casecadeIndex,  // カスケード番号.
    float           depth           // シャドウマップと比較したい深度値.
)
{
    return shadowMap.GatherCmpRed(
        shadowSmp, float3(coord, cascadeIndex), depth);
}

#else
//-----------------------------------------------------------------------------
//      シャドウマップをサンプルします(ピクセルシェーダ用).
//-----------------------------------------------------------------------------
float4 SampleShadow
(
    Texture2DArray  shadowMap,      // カスケードシャドウマップ.
    ShadowSampler   shadowSmp,      // シャドウマップサンプラー.
    float2          coord,          // シャドウマップ座標.
    uint            cascadeIndex,   // カスケード番号.
    float           depth           // シャドウマップと比較したい深度値.
)
{
    return shadowMap.SampleCmpLevelZero(
        shadowSmp, float3(coord, cascadeIndex), depth);
}

#endif//ASDX_COMPUTE_SHADER

//-----------------------------------------------------------------------------
//      UNORMフォーマットでの定数バイアスを計算します.
//-----------------------------------------------------------------------------
float ConstantBiasUnorm(float bias)
{ return bias * ASDX_SHADOW_EPSILON; }

//-----------------------------------------------------------------------------
//      FLOATフォーマットでの定数バイアスを計算します.
//-----------------------------------------------------------------------------
float ConstantBiasFloat(float z, float bias)
{ return bias * pow(2.0f, (exp(z) - ASDX_SHADOW_MANTISSA_BITS)); }

//-----------------------------------------------------------------------------
//      傾斜バイアスを計算します.
//------------------------------------------------------------------------------
float3 SlopeBias(float3 N, float3 L, float scale)
{ return L * saturate(1.0f - dot(N, L)) * scale; }

//-----------------------------------------------------------------------------
//      法線オフセットによる深度バイアス
//-----------------------------------------------------------------------------
float3 NormalOffsetBias(float3 pos, float3 N, float3 L, float offset_scale)
{
    float slope_scale = saturate(1.0 - dot(N, L));
    return pos + N * offset_scale * slope_scale;
}

//-----------------------------------------------------------------------------
//      PCF 1tap.
//-----------------------------------------------------------------------------
float SamplePCF1
(
    Texture2DArray  shadowMap,      // カスケードシャドウマップ.
    ShadowSampler   shadowSmp,      // シャドウサンプラー.
    float3          coord,          // (シャドウマップ行列 * テクスチャ行列)で変換した位置座標.
    uint            cascadeIndex    // カスケード番号.
)
{ return SampleShadow(shadowMap, shadowSmp, coord.xy, cascadeIndex, saturate(coord.z)); }

//-----------------------------------------------------------------------------
//      PCF 4tap.
//-----------------------------------------------------------------------------
float SamplePCF4
(
    Texture2DArray  shadowMap,      // カスケードシャドウマップ.
    ShadowSampler   shadowSmp,      // シャドウサンプラー.
    float3          coord,          // (シャドウマップ行列 * テクスチャ行列)で変換した位置座標.
    uint            cascadeIndex,   // カスケード番号.
    float2          shadowMapSize   // シャドウマップサイズ.
)
{
    float2 invMapSize = 1.0f / shadowMapSize;
    float2 uv = coord.xy * shadowMapSize + float2(0.5f, 0.5f);
    float2 baseUV = floor(uv);
    float2 st = uv.xy - baseUV.xy;

    baseUV -= float2(0.5f, 0.5f);
    baseUV *= invMapSize;

    float2 w0 = 3.0f - 2.0f * st;
    float2 w1 = 1.0f + 2.0f * st;

    float2 uv0 = (2.0f - st) / w0 - 1.0f;
    float2 uv1 = st / w1 + 1.0f;
    uv0 *= invMapSize;
    uv1 *= invMapSize;

    float depth  = saturate(coord.z);
    float result = 0.0f;
    result += (w0.x * w0.y) * SampleShadow(shadowMap, shadowSmp, baseUV + float2(uv0.x, uv0.y), cascadeIndex, depth).x;
    result += (w1.x * w0.y) * SampleShadow(shadowMap, shadowSmp, baseUV + float2(uv1.x, uv0.y), cascadeIndex, depth).x;
    result += (w0.x * w1.y) * SampleShadow(shadowMap, shadowSmp, baseUV + float2(uv0.x, uv1.y), cascadeIndex, depth).x;
    result += (w1.x * w1.y) * SampleShadow(shadowMap, shadowSmp, baseUV + float2(uv1.x, uv1.y), cascadeIndex, depth).x;

    return saturate(result * 0.0625); // 0.0625 = 1/16.
}

//-----------------------------------------------------------------------------
//      チェビシェフの不等式.
//-----------------------------------------------------------------------------
float Chebyshev(float2 moments, float mean, float minVariance)
{
    // 分散を計算します.
    float variance = moments.y - (moments.x * moments.x);
    variance = max(variance, minVariance);

    // 上限確率を計算します.
    float d = mean - moments.x;
    float prob = variance / (variance + (d * d));

    // 片側チェビシェフ不等式.
    return (mean <= moments.x) ? 1.0f : prob;
}

//-----------------------------------------------------------------------------
//      プロシージャルグリッドを描画します.
//-----------------------------------------------------------------------------
float3 DrawGrid
(
    float3 baseColor,         // 基本色.
    float2 shadowCoord,       // シャドウマップテクスチャ座標.
    float  gridLineWidth,     // グリッド線の太さ(ピクセル単位).
    float3 gridColor,         // グリッド線の色.
    float  spacing            // グリッド間隔(ピクセル単位).
)
{
    float2 st = shadowCoord / spacing;
    float2 dx = float2(ddx_fine(st.x), ddy_fine(st.x));
    float2 dy = float2(ddx_fine(st.y), ddy_fine(st.y));
    float2 m  = frac(st);

    if ( m.x < gridLineWidth * length(dx) 
      || m.y < gridLineWidth * length(dy))
    { return gridColor; }
    else
    { return baseColor; }
}

//-----------------------------------------------------------------------------
//      テクスチャを使って誤差を可視化します.
//-----------------------------------------------------------------------------
float3 VisualizeError
(
    Texture2D       errorMap,   // 誤差を表す色が格納されたテクスチャ.
    SamplerState    errorSmp,   // クランプサンプラー.
    float2          texcoord,   // シャドウマップテクスチャ座標.
    float2          mapSize     // シャドウマップサイズ.
)
{
    float2 ds = mapSize.x * ddx_fine(texcoord);
    float2 dt = mapSize.y * ddy_fine(texcoord);
    float s = 0.1f; // [0, 10.0]の値を[0, 1]にマッピングするので0.1倍.
    float error = max(length(ds + dt), length(ds - dt)) * s;
    return errorMap.Sample(errorSmp, error).rgb;
}

#endif // ASDX_SHADOW_HLSLI
