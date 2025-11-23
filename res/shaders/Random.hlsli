//-----------------------------------------------------------------------------
// File : Random.hlsli
// Desc : Random Number Generation Functions.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#ifndef ASDX_RANDOM_HLSLI
#define ASDX_RANDOM_HLSLI

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Math.hlsli"


// ディザ判定用テールブル.
static const float F_DITHER_LIST[4][4] =
{
    { 0.37647f, 0.87450f, 0.50196f, 0.99000f },
    { 0.62352f, 0.12549f, 0.75294f, 0.25098f },
    { 0.43921f, 0.93725f, 0.31372f, 0.81568f },
    { 0.68627f, 0.18823f, 0.56470f, 0.06274f },
};

//-----------------------------------------------------------------------------
//      疑似乱数を生成します.
//-----------------------------------------------------------------------------
float FracSin(float2 value)
{
    return frac(sin(dot(value.xy, float2(12.9898, 78.233))) * 43758.5453);
}

//-----------------------------------------------------------------------------
//      Hammersleyサンプリング.
//-----------------------------------------------------------------------------
float2 Hammersley(uint i, uint N)
{
    float ri = reversebits(i) * 2.3283064365386963e-10f;
    return float2(float(i) / float(N), ri);
}

//-----------------------------------------------------------------------------
//      R1数列.
//-----------------------------------------------------------------------------
float R1Sequence(float number)
{
    // Martin Roberts, "The Unreasonable Effectiveness of Quasirandom Sequences", 2018
    // http://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/

    // g  = 1.6180339887498948482;
    // a1 = 1 / g = 0.6180339887498948482
    return frac(0.5f + number * 0.6180339887498948482);
}

//-----------------------------------------------------------------------------
//      R2数列.
//-----------------------------------------------------------------------------
float2 R2Sequence(float number)
{
    // g = 1.32471795724474602596
    // a1 = 1 / g       = 0.75487766624669276005
    // a2 = 1 / (g * g) = 0.5698402909980532659114;
    return frac(0.5f.xx + number * float2(0.75487766624669276005, 0.5698402909980532659114));
}

//-----------------------------------------------------------------------------
//      三角ノイズを計算します.
//-----------------------------------------------------------------------------
float TriangleNoise(float2 n, float time)
{
    // triangle noise, in [-1.0..1.0[ range
    float v = 0.07 * frac(time);
    n += float2(v, v);
    n = frac(n * float2(5.3987, 5.4421));
    n += dot(n.yx, n.xy + float2(21.5351, 14.3137));

    float xy = n.x * n.y;
    // compute in [0..2[ and remap to [-1.0..1.0[
    return frac(xy * 95.4307) + frac(xy * 75.04961) - 1.0;
}

//-----------------------------------------------------------------------------
//      グラディエントノイズを計算します.
//-----------------------------------------------------------------------------
float InterleavedGradientNoise(const float2 n)
{
    return frac(52.982919 * frac(dot(float2(0.06711, 0.00584), n)));
}

//-----------------------------------------------------------------------------
//      バリューノイズを生成します.
//-----------------------------------------------------------------------------
float ValueNoise(float2 p)
{
    float2 i = floor(p);
    float2 f = frac(p);
    
    float2 s = smoothstep(0.0, 1.0, f);
    float nx0 = lerp(FracSin(i + float2(0.0, 0.0)), FracSin(i + float2(1.0, 0.0)), s.x);
    float nx1 = lerp(FracSin(i + float2(0.0, 1.0)), FracSin(i + float2(1.0, 1.0)), s.x);
    return lerp(nx0, nx1, s.y);
}

//-----------------------------------------------------------------------------
//      半球を一様サンプリングします.
//-----------------------------------------------------------------------------
float3 UniformSampleHemisphere(float2 u)
{
    float z = u.x;
    float r = sqrt(max(0.0f, 1.0f - z * z));
    float phi = 2.0f * F_PI * u.y;

    return float3(r * cos(phi), r * sin(phi), z);
    // Pdf = z * (1.0f / F_PI);
}

//-----------------------------------------------------------------------------
//      全球を一様サンプリングします.
//-----------------------------------------------------------------------------
float3 UniformSampleSphere(float2 u)
{
    float z = 1.0f - 2.0f * u.x;
    float r = sqrt(max(0.0f, 1.0f - z * z));
    float phi = 2.0f * F_PI * u.y;

    return float3(r * cos(phi), r * sin(phi), z);
    // Pdf = 1.0f / (4.0f * F_PI);
}

//-----------------------------------------------------------------------------
//      円盤を一様サンプリングします.
//-----------------------------------------------------------------------------
float2 UniformSampleDisk(float2 u)
{
    float r = sqrt(u.x);
    float theta = 2.0f * F_PI * u.y;

    return float2(r * cos(theta), r * sin(theta));
}

//-----------------------------------------------------------------------------
//      円錐を一様サンプリングします.
//-----------------------------------------------------------------------------
float3 UniformSampleCone(float2 u, float cosThetaMax)
{
    float cosTheta = (1.0f - u.x) + u.x * cosThetaMax;
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
    float phi = u.y * 2.0f * F_PI;

    return float3(
        cos(phi) * sinTheta,
        sin(phi) * sinTheta,
        cosTheta);
}

//-----------------------------------------------------------------------------
//      三角形を一様サンプリングします.
//-----------------------------------------------------------------------------
float3 UniformSampleTriangle(float2 u)
{
    float su = sqrt(u.x);
    float2 b = float2(1.0f - su, u.y * su);
    return float3(1.0f - b.x - b.y, b.x, b.y);
}

//-----------------------------------------------------------------------------
//      乱数の種を設定します.
//-----------------------------------------------------------------------------
uint4 SetSeed(uint2 pixelCoords, uint frameIndex)
{ return uint4(pixelCoords, frameIndex, 0); }

//-----------------------------------------------------------------------------
//      Permuted Congruential Generator (PCG)
//-----------------------------------------------------------------------------
uint4 PCG(uint4 v)
{
    v = v * 1664525u + 101390422u;

    v.x += v.y * v.w;
    v.y += v.z * v.x;
    v.z += v.x * v.y;
    v.w += v.y * v.z;

    v = v ^ (v >> 16u);
    v.x += v.y * v.w;
    v.y += v.z * v.x;
    v.z += v.x * v.y;
    v.w += v.y * v.z;

    return v;
}

//-----------------------------------------------------------------------------
//      PCGによる疑似乱数を生成します.
//-----------------------------------------------------------------------------
float FloatPCG(inout uint4 seed)
{
    seed.w++;
    return PCG(seed).x * 2.3283064365386962890625e-10; // 浮動小数にする.
}

//-----------------------------------------------------------------------------
//      IbukiHashによる疑似乱数を生成します.
//-----------------------------------------------------------------------------
float Ibuki(inout uint4 u)
{
    // IbukiHash by Andante
    // This work is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

    // [Andante 2024] Andaten, "屋根裏工房改: 高速で頑健なシェーダー乱数の比較と提案", 2024.
    // https://andantesoft.hatenablog.com/entry/2024/12/19/193517
    const uint4 mult = uint4(0xae3cc725, 0x9fe72885, 0xae36bfb5, 0x82c1fcad);

    u = u * mult;
    u ^= u.wxyz ^ u >> 13;

    uint r = dot(u, mult);

    r ^= r >> 11;
    r = (r * r) ^ r;

    return r * 2.3283064365386962890625e-10; // 浮動小数にする.
}

//-----------------------------------------------------------------------------
//      ディザ処理.
//-----------------------------------------------------------------------------
void Dithering(float2 sv_position, float alpha)
{
    uint2 screenPos = (uint2) fmod(sv_position, 4.0f);
    if (alpha < F_DITHER_LIST[screenPos.x][screenPos.y])
    {
        discard;
    }
}

//-----------------------------------------------------------------------------
//      Jimenezによるディザーを計算します.
//-----------------------------------------------------------------------------
float4 DitherJimenez(float2 uv, float time, float4 rgba)
{
    // Jimenez 2014, "Next Generation Post-Processing in Call of Duty"
    float noise = InterleavedGradientNoise(uv.xy + time);
    // remap from [0..1[ to [-1..1[
    noise = (noise * 2.0) - 1.0;
    return float4(rgba.rgb + noise / 255.0, rgba.a);
}

//------------------------------------------------------------------------------
//      Gj?lによるディザーを計算します.
//------------------------------------------------------------------------------
float4 DitherTriangleNoise(float4 rgba, float2 uv, float2 screenSize, float time)
{
    // Gj?l 2016, "Banding in Games: A Noisy Rant", http://loopit.dk/banding_in_games.pdf.
    return rgba + TriangleNoise(uv * screenSize, time) / 255.0;
}

//-----------------------------------------------------------------------------
//      Gj?lによるRGBディザーを計算します.
//-----------------------------------------------------------------------------
float4 DitherTriangleNoiseRGB(float4 rgba, float2 uv, float2 screenSize, float time)
{
    // Gj?l 2016, "Banding in Games: A Noisy Rant", http://loopit.dk/banding_in_games.pdf.
    float2 st = uv * screenSize;
    float3 dither = float3(
            TriangleNoise(st, time),
            TriangleNoise(st + 0.1337, time),
            TriangleNoise(st + 0.3141, time)) / 255.0;
    return float4(rgba.rgb + dither, rgba.a + dither.x);
}

#endif//ASDX_RANDOM_HLSLI
