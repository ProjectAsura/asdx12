//-----------------------------------------------------------------------------
// File : TaaCS.hlsl
// Desc : Temporal Anti-Alias.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Math.hlsli"

#ifndef ENABLE_REVERSE_Z
#define ENABLE_REVERSE_Z    (0)
#endif//ENABLE_REVERSE_Z

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static const float kVarianceIntersectionMaxT  = 100.0f;
static const float kFrameVelocityInPixelsDiff = 256.0f; // 1920 x 1080.


///////////////////////////////////////////////////////////////////////////////
// CbTemporalAA constant buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbTemporalAA : register(b0)
{
    float   Gamma;          // 1.0でうまく動くとのこと. [0.75, 1.25]で設定するのがGoodとのこと.
    float   BlendFactor;
    float2  MapSize;
    float2  InvMapSize;
    float2  Jitter;
};


//-----------------------------------------------------------------------------
// Textures and Samplers.
//-----------------------------------------------------------------------------
Texture2D           ColorMap        : register(t0);
Texture2D           HistoryMap      : register(t1);
Texture2D<float2>   VelocityMap     : register(t2);
Texture2D<float>    DepthMap        : register(t3);
RWTexture2D<float4> OutColorMap     : register(u0);
RWTexture2D<float4> OutHistoryMap   : register(u1);
SamplerState        PointClamp      : register(s1); // Samplers.hlsliと合わせている.
SamplerState        LinearClamp     : register(s4); // Samplers.hlsliと合わせている.

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static const int2 kOffsets[8] = {
    int2(-1, -1),
    int2(-1,  1),
    int2( 1, -1),
    int2( 1,  1),
    int2( 1,  0),
    int2( 0, -1),
    int2( 0,  1),
    int2(-1,  0)
};


//-----------------------------------------------------------------------------
//      RGBからYCoCgに変換します.
//-----------------------------------------------------------------------------
float3 RGBToYCoCg( float3 RGB )
{
    float Y  = dot(RGB, float3(  1, 2,  1 )) * 0.25;
    float Co = dot(RGB, float3(  2, 0, -2 )) * 0.25 + ( 0.5 * 256.0/255.0 );
    float Cg = dot(RGB, float3( -1, 2, -1 )) * 0.25 + ( 0.5 * 256.0/255.0 );
    return float3(Y, Co, Cg);
}

//-----------------------------------------------------------------------------
//      YCoCgからRGBに変換します.
//-----------------------------------------------------------------------------
float3 YCoCgToRGB( float3 YCoCg )
{
    float Y  = YCoCg.x;
    float Co = YCoCg.y - ( 0.5 * 256.0 / 255.0 );
    float Cg = YCoCg.z - ( 0.5 * 256.0 / 255.0 );
    float R  = Y + Co-Cg;
    float G  = Y + Cg;
    float B  = Y - Co-Cg;
    return float3(R, G, B);
}

//-----------------------------------------------------------------------------
//      Catmull-Rom フィルタリング.
//-----------------------------------------------------------------------------
float4 BicubicSampleCatmullRom(Texture2D map, SamplerState smp, float2 uv, float2 mapSize)
{
    float2 samplePos = uv * MapSize;
    float2 tc = floor(samplePos - 0.5f) + 0.5f;
    float2 f  = samplePos - tc;
    float2 f2 = f * f;
    float2 f3 = f * f2;

    float2 w0 = f2 - 0.5f * (f3 + f);
    float2 w1 = 1.5f * f3 - 2.5f * f2 + 1;
    float2 w3 = 0.5f * (f3 - f2);
    float2 w2 = 1 - w0 - w1 - w3;

    float2 w12 = w1 + w2;

    float2 tc0  = (tc - 1) * InvMapSize;
    float2 tc12 = (tc + w2 / w12) * InvMapSize;
    float2 tc3  = (tc + 2) * InvMapSize;

    float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);

    result += map.SampleLevel(smp, float2(tc0.x,  tc0.y),  0) * (w0.x  * w0.y);
    result += map.SampleLevel(smp, float2(tc0.x,  tc12.y), 0) * (w0.x  * w12.y);
    result += map.SampleLevel(smp, float2(tc0.x,  tc3.y),  0) * (w0.x  * w3.y);

    result += map.SampleLevel(smp, float2(tc12.x, tc0.y),  0) * (w12.x * w0.y);
    result += map.SampleLevel(smp, float2(tc12.x, tc12.y), 0) * (w12.x * w12.y);
    result += map.SampleLevel(smp, float2(tc12.x, tc3.y),  0) * (w12.x * w3.y);

    result += map.SampleLevel(smp, float2(tc3.x,  tc0.y),  0) * (w3.x * w0.y);
    result += map.SampleLevel(smp, float2(tc3.x,  tc12.y), 0) * (w3.x * w12.y);
    result += map.SampleLevel(smp, float2(tc3.x,  tc3.y),  0) * (w3.x * w3.y);

    return max(result, 0.0f.xxxx);
}

//-----------------------------------------------------------------------------
//      現在フレームのカラーを取得します.
//-----------------------------------------------------------------------------
float4 GetCurrentColor(float2 uv)
{ return ColorMap.SampleLevel(PointClamp, uv, 0.0f); }

//-----------------------------------------------------------------------------
//      ヒストリーカラーを取得します.
//-----------------------------------------------------------------------------
float4 GetHistoryColor(float2 uv)
{ return BicubicSampleCatmullRom(HistoryMap, LinearClamp, uv, MapSize); }

//-----------------------------------------------------------------------------
//      速度ベクトルを取得します.
//-----------------------------------------------------------------------------
float2 GetVelocity(float2 uv)
{
    float2 result = VelocityMap.SampleLevel(PointClamp, uv, 0.0f);
    float  currLengthSq = dot(result, result);

    // 最も長い速度ベクトルを取得.
    [unroll] for(uint i=0; i<8; ++i)
    {
        float2 velocity = VelocityMap.SampleLevel(PointClamp, uv, 0.0f, kOffsets[i]);
        float  lengthSq = dot(velocity, velocity);
        if (lengthSq > currLengthSq)
        {
            result       = velocity;
            currLengthSq = lengthSq;
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
//      隣接ピクセルを考慮して深度を取得します.
//-----------------------------------------------------------------------------
float GetCurrentDepth(float2 uv)
{
    // 隣接4x4ピクセルをチェックし，最も手前のものを返却.
    float4 depths = DepthMap.GatherRed(PointClamp, uv);
    #if ENABLE_REVERSE_Z
        return max( max(depths.x, depths.y), max(depths.z, depths.w) );
    #else
        return min( min(depths.x, depths.y), min(depths.z, depths.w) );
    #endif
}

//-----------------------------------------------------------------------------
//      隣接ピクセルを考慮して深度を取得します.
//-----------------------------------------------------------------------------
float GetPreviousDepth(float2 uv)
{
    // CurrentDepthと逆演算.
    float4 depths = DepthMap.GatherRed(PointClamp, uv);
    #if ENABLE_REVERSE_Z
        return min( max(depths.x, depths.y), min(depths.z, depths.w) ) + 0.001f;
    #else
        return max( min(depths.x, depths.y), max(depths.z, depths.w) ) + 0.001f;
    #endif
}

//-----------------------------------------------------------------------------
//      ヒストリーカラーをクリップするためのバウンディングボックスを求めます.
//-----------------------------------------------------------------------------
void CalcColorBoundingBox
(
    float2      uv,
    float3      curColor,
    float       gamma,
    out float3  minColor,
    out float3  maxColor
)
{
    // 平均と分散を求める.
    float3 ave = curColor;
    float3 var = curColor * curColor;

    [unroll] for(uint i=0; i<8; ++i)
    {
        float3 newColor = RGBToYCoCg(ColorMap.SampleLevel(PointClamp, uv, 0.0f, kOffsets[i]).rgb);
        ave += newColor;
        var += newColor * newColor;
    }

    const float invSamples = 1.0f / 9.0f;
    ave *= invSamples;
    var *= invSamples;
    var = sqrt(var - ave * ave) * gamma;

    // 分散クリッピング.
    minColor = float3(ave - var);
    maxColor = float3(ave + var);
}

//-----------------------------------------------------------------------------
//      最終カラーを求めます.
//-----------------------------------------------------------------------------
float4 GetFinalColor(float2 uv, float3 currentColor, float3 historyColor, float weight)
{
    float newWeight = saturate(0.5f - weight);
    return float4(lerp(currentColor, historyColor, saturate(BlendFactor)), newWeight);
}

//-----------------------------------------------------------------------------
//      隣接ピクセルを考慮した現在カラーを取得します.
//-----------------------------------------------------------------------------
float3 GetCurrentNeighborColor(float2 uv, float3 currentColor)
{
    const float centerWeight = 4.0f;
    float3 accColor = currentColor * centerWeight;
    [unroll] 
    for(uint i=0; i<4; ++i)
    {
        accColor += ColorMap.SampleLevel(PointClamp, uv, 0.0f, kOffsets[i]).rgb;
    }
    const float invWeight = 1.0f / (4.0f + centerWeight);
    accColor *= invWeight;
    return accColor;
}

//-----------------------------------------------------------------------------
//      重みを求めます.
//-----------------------------------------------------------------------------
float CalcHdrWeightY(float3 ycocg, float exposure = 1.0f)
{ return rcp(ycocg.x * exposure + 4.0f); }

//-----------------------------------------------------------------------------
//      ブレンドウェイトを求めます.
//-----------------------------------------------------------------------------
float2 CalcBlendWeight(float historyWeight, float currentWeight, float blend)
{
    float blendH = (1.0f - blend) * historyWeight;
    float blendC = blend * currentWeight;
    return float2(blendH, blendC) * rcp(blendH + blendC);
}

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID)
{
    if (any(dispatchId.xy >= (uint2)MapSize))
    { return; }

    const float2 currUV = ((float2)dispatchId.xy + 0.5f.xx) * InvMapSize;

    // 現在フレームのカラー.
    float4 currColor = GetCurrentColor(currUV);
    
    // 1920x1080をベースとしたスケール.
    const float sizeScale = MapSize.x / 1920.0f;

    // 速度ベクトルを取得.
    float2 velocity = GetVelocity(currUV);
    float  velocityDelta = saturate(1.0f - length(velocity) / (kFrameVelocityInPixelsDiff * sizeScale));

    // 前フレームのテクスチャ座標を計算.
    float2 prevUV = currUV + (velocity * InvMapSize);

    // 深度値を取得.
    float currDepth  = GetCurrentDepth(currUV);
    float prevDepth  = GetPreviousDepth(prevUV + Jitter);
    float depthDelta = step(currDepth, prevDepth);

    // 画面内かどうか?
    float inScreen = (all(0.0f.xx <= prevUV) && all(prevUV < 1.0f.xx)) ? 1.0f : 0.0f;

    // ヒストリーが有効かどうかチェックする.
    // 速度の差分，深度の差分，画面範囲であるか，いずれかすくなくとも１つがゼロなら無効と判断.
    bool isValidHistory = (velocityDelta * depthDelta * inScreen) > 0.0f;

    float4 finalColor;

    // ヒストリーバッファが有効な場合.
    [branch]
    if (isValidHistory)
    {
        // ヒストリーカラーを取得.
        float4 prevColor = GetHistoryColor(prevUV);

        // YCoCgに変換.
        currColor.rgb = RGBToYCoCg(currColor.rgb);
        prevColor.rgb = RGBToYCoCg(prevColor.rgb);

        // クリップ済みヒストリーカラーを取得する.
        float3 minColor, maxColor;
        CalcColorBoundingBox(prevUV, currColor.rgb, Gamma, minColor, maxColor);

        // AABBでクリップ.
        float t = IntersectAABB(currColor.rgb - prevColor.rgb, prevColor.rgb, minColor, maxColor);
        prevColor.rgb = lerp(prevColor.rgb, currColor.rgb, saturate(t));
        prevColor.rgb = clamp(prevColor.rgb, minColor, maxColor);

        // 重みを求める.
        float blend = saturate(1.0f - BlendFactor);
        blend = max(blend, saturate(0.01f * prevColor.x / abs(currColor.x - prevColor.x)));
        float  currWeight = CalcHdrWeightY(currColor.rgb);
        float  prevWeight = CalcHdrWeightY(prevColor.rgb);
        float2 weights    = CalcBlendWeight(prevWeight, currWeight, saturate(blend));

        // 補間処理.
        finalColor = prevColor * weights.x + currColor * weights.y;
        
        // RGBに戻す.
        finalColor.rgb = YCoCgToRGB(finalColor.rgb);
    }
    else
    {
        // ヒストリーバッファが無効な場合は隣接ピクセルを考慮して最終カラーを求める.
        float3 neighborColor = GetCurrentNeighborColor(currUV, currColor.rgb);
        finalColor = float4(neighborColor, 0.5f);
    }

    // NaNを潰しておく.
    finalColor = SaturateFloat(finalColor);

    // 出力.
    OutColorMap  [dispatchId.xy] = float4(finalColor.rgb, 1.0f);
    OutHistoryMap[dispatchId.xy] = finalColor;
}