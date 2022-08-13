﻿//-----------------------------------------------------------------------------
// File : TemporalAA_PS.hlsl
// Desc : Temporal Anti-Alias.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

#ifndef ENABLE_REVERSE_Z
#define ENABLE_REVERSE_Z    (0)
#endif//ENABLE_REVERSE_Z


//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static const float kVarianceIntersectionMaxT  = 100.0f;
static const float kFrameVelocityInPixelsDiff = 256.0f; // 1920 x 1080.


///////////////////////////////////////////////////////////////////////////////
// VSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

///////////////////////////////////////////////////////////////////////////////
// PSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct PSOutput
{
    float4 Color   : SV_TARGET0;
};

///////////////////////////////////////////////////////////////////////////////
// CbTemporalAA constant buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbTemporalAA : register(b0)
{
    float   Gamma;          // 1.0でうまく動くとのこと. [0.75, 1.25]で設定するのがGoodとのこと.
    float   BlendFactor;
    float2  MapSize;
    float2  CurrJitter;
    float2  PrevJitter;
};


//-----------------------------------------------------------------------------
// Textures and Samplers.
//-----------------------------------------------------------------------------
Texture2D           ColorMap    : register(t0);
Texture2D           HistoryMap  : register(t1);
Texture2D<float2>   VelocityMap : register(t2);
Texture2D<float>    DepthMap    : register(t3);
SamplerState        PointClamp  : register(s0);
SamplerState        LinearClamp : register(s1);

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static const int2 kOffsets[8] = {
    int2(-1,- 1), int2(-1,  1),
    int2( 1, -1), int2( 1,  1),
    int2( 1,  0), int2( 0, -1),
    int2( 0,  1), int2(-1,  0)
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
    return float3(R,G,B);
}

//-----------------------------------------------------------------------------
//      Catmull-Rom フィルタリング.
//-----------------------------------------------------------------------------
float4 BicubicSampleCatmullRom(Texture2D map, SamplerState smp, float2 uv, float2 mapSize)
{
    float2 samplePos = uv * mapSize;
    float2 tc = floor(samplePos - 0.5f) + 0.5f;
    float2 f  = samplePos - tc;
    float2 f2 = f * f;
    float2 f3 = f * f2;

    float2 w0 = f2 - 0.5f * (f3 + f);
    float2 w1 = 1.5f * f3 - 2.5f * f2 + 1;
    float2 w3 = 0.5f * (f3 - f2);
    float2 w2 = 1 - w0 - w1 - w3;

    float2 w12 = w1 + w2;

    float2 invMapSize = 1.0f / mapSize;
    float2 tc0  = (tc - 1) * invMapSize;
    float2 tc12 = (tc + w2 / w12) * invMapSize;
    float2 tc3  = (tc + 2) * invMapSize;

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
float3 GetCurrentColor(float2 uv)
{ return ColorMap.SampleLevel(PointClamp, uv, 0.0f).rgb; }

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
float GetDepth(float2 uv)
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
//      カラーをAABBでクリップします.
//-----------------------------------------------------------------------------
float3 ClipToAABB(float3 currentColor, float3 historyColor, float3 center, float3 extents)
{
    // 方向ベクトルを求める.
    float3 dir = currentColor - historyColor;

    // 交差位置を求める.
    float3 intersection = ((center - sign(dir) * extents) - historyColor) / dir;
    float3 possibleT = (intersection >= 0.0f.xxx) ? intersection : kVarianceIntersectionMaxT  + 1.0f;
    float  t = min(kVarianceIntersectionMaxT, min(possibleT.x, min(possibleT.y, possibleT.z)));
    return (t < kVarianceIntersectionMaxT) ? historyColor + dir * t : historyColor;
}

//-----------------------------------------------------------------------------
//      ヒストリーカラーをクリップします.
//-----------------------------------------------------------------------------
float3 ClipHistoryColor(float2 uv, float3 currentColor, float3 historyColor, float gamma)
{
    const float3 currentColorYCoCg = RGBToYCoCg(currentColor);

    // 平均と分散を求める.
    float3 ave = currentColorYCoCg;
    float3 var = currentColorYCoCg * currentColorYCoCg;

    [unroll] for(uint i=0; i<8; ++i)
    {
        float3 newColorYCoCg = RGBToYCoCg(ColorMap.SampleLevel(PointClamp, uv, 0.0f, kOffsets[i]).rgb);
        ave += newColorYCoCg;
        var += newColorYCoCg * newColorYCoCg;
    }

    const float invSamples = 1.0f / 9.0f;
    ave *= invSamples;
    var = sqrt(var * invSamples - ave * ave) * gamma;

    return YCoCgToRGB(ClipToAABB(YCoCgToRGB(historyColor), currentColorYCoCg, ave, var));
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
    [unroll] for(uint i=0; i<4; ++i)
    {
        accColor += ColorMap.SampleLevel(PointClamp, uv, 0.0f, kOffsets[i]).rgb;
    }
    const float invWeight = 1.0f / (4.0f + centerWeight);
    accColor *= invWeight;
    return accColor;
}

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
PSOutput main(const VSOutput input)
{
    const float2 currUV = input.TexCoord;

    // 現在フレームのカラー.
    float3 currColor = GetCurrentColor(currUV + CurrJitter);
    
    // 1920x1080をベースとしたスケール.
    const float sizeScale = MapSize.x / 1920.0f;

    // 速度ベクトルを取得.
    float2 velocity = GetVelocity(currUV);
    float  velocityDelta = saturate(1.0f - length(velocity) / (kFrameVelocityInPixelsDiff * sizeScale));

    // 前フレームのテクスチャ座標を計算.
    float2 prevUV = ((currUV + PrevJitter) + velocity / MapSize);

    // 深度値を取得.
    float currDepth = GetDepth(currUV);
    float prevDepth = GetDepth(prevUV);
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
        // 生のヒストリーカラーを取得.
        float4 rawHistoryColor = GetHistoryColor(prevUV);

        // クリップ済みヒストリーカラーを取得する.
        float3 historyColor = ClipHistoryColor(prevUV, currColor, rawHistoryColor.rgb, Gamma);

        // 重みを求める.
        float weight = rawHistoryColor.a * velocityDelta * depthDelta;

        // 現在カラーとヒストリーカラーをブレンドして最終カラーを求める.
        finalColor = GetFinalColor(currUV, currColor, historyColor, weight);
    }
    else
    {
        // ヒストリーバッファが無効な場合は隣接ピクセルを考慮して最終カラーを求める.
        float3 neighborColor = GetCurrentNeighborColor(currUV, currColor);
        finalColor = float4(neighborColor, 0.5f);
    }

    // 出力データ作成.
    PSOutput output = (PSOutput)0;
    output.Color = finalColor;

    // 出力.
    return output;
}