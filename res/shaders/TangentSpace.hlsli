//-----------------------------------------------------------------------------
// File : TangentSpace.hlsli
// Desc : Tangent Space Utilities.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#ifndef ASDX_TANGENT_SPACE_HLSLI
#define ASDX_TANGENT_SPACE_HLSLI

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Math.hlsli"


//-----------------------------------------------------------------------------
//      接線空間からワールド空間に変換します.
//-----------------------------------------------------------------------------
float3 FromTangentSpaceToWorld(float3 value, float3 T, float3 B, float3 N)
{
    return normalize(value.x * T + value.y * B + value.z * N);
}

//-----------------------------------------------------------------------------
//      八面体ラップ処理を行います.
//-----------------------------------------------------------------------------
float2 OctWrap(float2 v)
{
#if __HLSL_VERSION >= 2021
    return (1.0f - abs(v.yx)) * select(v.xy >= 0.0f, 1.0f, -1.0f);
#else
    return (1.0f - abs(v.yx)) * (v.xy >= 0.0f ? 1.0f : -1.0f);
#endif
}

//-----------------------------------------------------------------------------
//      法線ベクトルをパッキングします.
//-----------------------------------------------------------------------------
float2 PackNormal(float3 normal)
{
    // Octahedron normal vector encoding.
    // https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/
    float3 n = normal / (abs(normal.x) + abs(normal.y) + abs(normal.z));
#if __HLSL_VERSION >= 2021
    n.xy = select(n.z >= 0.0f, n.xy, OctWrap(n.xy));
#else
    n.xy = (n.z >= 0.0f) ? n.xy : OctWrap(n.xy);
#endif
    return n.xy * 0.5f + 0.5f;
}

//-----------------------------------------------------------------------------
//      法線ベクトルをアンパッキングします.
//-----------------------------------------------------------------------------
float3 UnpackNormal(float2 packed)
{
    // Octahedron normal vector encoding.
    // https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/
    float2 encoded = packed * 2.0f - 1.0f;
    float3 n = float3(encoded.x, encoded.y, 1.0f - abs(encoded.x) - abs(encoded.y));
    float t = saturate(-n.z);
#if __HLSL_VERSION >= 2021
    n.xy += select(n.xy >= 0.0f, -t, t);
#else
    n.xy += (n.xy >= 0.0f) ? -t : t;
#endif
    return normalize(n);
}

//-----------------------------------------------------------------------------
//      最小となる差分値を求めます.
//-----------------------------------------------------------------------------
float3 MinDiff(float3 p, float3 r, float3 l)
{
    float3 v1 = r - p;
    float3 v2 = p - l;
#if __HLSL_VERSION >= 2021
    return select(dot(v1, v1) < dot(v2, v2),  v1, v2);
#else
    return (dot(v1, v1) < dot(v2, v2)) ? v1 : v2;
#endif
}

//-----------------------------------------------------------------------------
//      法線ベクトルを再構築します.
//-----------------------------------------------------------------------------
float3 ToNormal(float3 p0, float3 pr, float3 pl, float3 pt, float3 pb)
{
    // p0 : 中心位置.
    // pr : p0 + (1, 0);
    // pl : p0 - (1, 0);
    // pt : p0 + (0, 1);
    // pb : p0 - (0, 1);
    return normalize(cross(MinDiff(p0, pr, pl), MinDiff(p0, pt, pb)));
}

//-----------------------------------------------------------------------------
//      法線マップを合成します.
//-----------------------------------------------------------------------------
float3 BlendNormal(float3 n1, float3 n2)
{
    float3 t = n1 * float3(2.0f, 2.0f, 2.0f) + float3(-1.0f, -1.0f, 0.0f);
    float3 u = n2 * float3(-2.0f, -2.0f, 2.0f) + float3(1.0f, 1.0f, -1.0f);
    float3 r = t * dot(t, u) - u * t.z;
    return normalize(r);
}

//-----------------------------------------------------------------------------
//      ベント反射ベクトルを計算します.
//-----------------------------------------------------------------------------
float3 BentReflection(float3 T, float3 B, float3 N, float3 V, float anisotropy)
{
#if __HLSL_VERSION >= 2021
    float3 anisotropicDirection = select(anisotropy >= 0.0f,  B, T);
#else
    float3 anisotropicDirection = (anisotropy >= 0.0f) ? B : T;
#endif
    float3 anisotropicTangent = cross(anisotropicDirection, V);
    float3 anisotropicNormal = cross(anisotropicTangent, anisotropicDirection);
    float3 bentNormal = normalize(lerp(N, anisotropicNormal, anisotropy));
    return reflect(V, bentNormal);
}


//-----------------------------------------------------------------------------
//      接線を再計算します.
//-----------------------------------------------------------------------------
float3 RecalcTangent(float3 normalMappedN, float3 T)
{
    // Johon Isidoro, Chris Brenman, "Per-Pixel Strand Based Anisotropic Lighting",
    // Direct3D ShaderX Vertex and Pixel Shader Tips and Tricks, pp.376-382, Wordware Publishing Inc.
    return normalize(T - dot(T, normalMappedN) * normalMappedN);
}


//-----------------------------------------------------------------------------
//      正規直交基底を求めます.
//-----------------------------------------------------------------------------
void CalcONB(float3 N, out float3 T, out float3 B)
{
    float sig = sign(N.z);
    float a = -1.0f / (sig + N.z);
    float b = N.x * N.y * a;
    T = float3(1.0f + sig * N.x * N.x * a, sig * b, -sig * N.x);
    B = float3(b, sig + N.y * N.y * a, -N.y);
}

//-----------------------------------------------------------------------------
//      パッキングされた接線空間を展開します.
//-----------------------------------------------------------------------------
void UnpackTN
(
    in uint encodedTBN, // 圧縮している接線空間(32bit).
    out float3 tangent, // 接線ベクトル.
    out float3 normal // 法線ベクトル.
)
{
    // Hawar Doghramachi and Jean-Normand Bucci, 
    // "Deferred+: Next-Gen Culling and Rendering for the Dawn Engine"
    // GPU Zen, pp.89-91, 2017.
    uint2 packedN;
    packedN.x = encodedTBN & 0x3FF;
    packedN.y = (encodedTBN >> 10) & 0x3FF;

    // octahedron normal vector decoding.
    normal = UnpackNormal(packedN / 1023.0f);
    
    uint cosAngleUint = (encodedTBN >> 20) & 0xFF;
    uint compIndex = (encodedTBN >> 28) & 0x3;

    // get reference vector
    float3 refVector;
    if (compIndex == 0)
    {
        refVector = float3(1.0f, 0.0f, 0.0f);
    }
    else if (compIndex == 1)
    {
        refVector = float3(0.0f, 1.0f, 0.0f);
    }
    else
    {
        refVector = float3(0.0f, 0.0f, 1.0f);
    }

    // decode tangent
    float cosAngle = (float(cosAngleUint) / 255.0f) * 2.0f - 1.0f;
    float sinAngle = sqrt(saturate(1.0f - (cosAngle * cosAngle)));
    
    uint handedness = (encodedTBN >> 30) & 0x3;

    sinAngle = ((handedness & 0x2) == 0) ? -sinAngle : sinAngle;
    float3 orthoA = normalize(cross(normal, refVector));
    float3 orthoB = cross(normal, orthoA);
    tangent = normalize((cosAngle * orthoA) + (sinAngle * orthoB));
}

//-----------------------------------------------------------------------------
//      パッキングされた接線空間を展開します.
//-----------------------------------------------------------------------------
void UnpackTBN
(
    in uint encodedTBN, // 圧縮している接線空間(32bit).
    out float3 tangent, // 接線ベクトル.
    out float3 bitangent, // 従接線ベクトル.
    out float3 normal // 法線ベクトル.
)
{
    // Hawar Doghramachi and Jean-Normand Bucci, 
    // "Deferred+: Next-Gen Culling and Rendering for the Dawn Engine"
    // GPU Zen, pp.89-91, 2017.
    uint2 packedN;
    packedN.x = encodedTBN & 0x3FF;
    packedN.y = (encodedTBN >> 10) & 0x3FF;

    // octahedron normal vector decoding.
    normal = UnpackNormal(packedN / 1023.0f);
    
    uint cosAngleUint = (encodedTBN >> 20) & 0xFF;
    uint compIndex = (encodedTBN >> 28) & 0x3;

    // get reference vector
    float3 refVector;
    if (compIndex == 0)
    {
        refVector = float3(1.0f, 0.0f, 0.0f);
    }
    else if (compIndex == 1)
    {
        refVector = float3(0.0f, 1.0f, 0.0f);
    }
    else
    {
        refVector = float3(0.0f, 0.0f, 1.0f);
    }

    // decode tangent
    float cosAngle = (float(cosAngleUint) / 255.0f) * 2.0f - 1.0f;
    float sinAngle = sqrt(saturate(1.0f - (cosAngle * cosAngle)));
    
    uint handedness = (encodedTBN >> 30) & 0x3;

    sinAngle = ((handedness & 0x2) == 0) ? -sinAngle : sinAngle;
    float3 orthoA = normalize(cross(normal, refVector));
    float3 orthoB = cross(normal, orthoA);
    tangent = normalize((cosAngle * orthoA) + (sinAngle * orthoB));

    // decode bitangent
    bitangent = cross(normal, tangent);
    bitangent = ((handedness & 0x1) == 0) ? bitangent : -bitangent;
}

//-----------------------------------------------------------------------------
//      接線空間を32bitにパッキングします.
//-----------------------------------------------------------------------------
uint PackTBN(float3 normal, float3 tangent, uint binormalHandedness)
{
    // Hawar Doghramachi and Jean-Normand Bucci, 
    // "Deferred+: Next-Gen Culling and Rendering for the Dawn Engine"
    // GPU Zen, pp.89-91, 2017.
    uint result = 0;
    float2 packNormal = PackNormal(normal);
    result |= uint(packNormal.x * 1023.0f);
    result |= uint(packNormal.y * 1023.0f) << 10;

    float3 tangentAbs = abs(tangent);
    float maxComp = Max3(tangentAbs);

    float3 refVector;
    uint compIndex = 0;
    if (maxComp == tangentAbs.x)
    {
        refVector = float3(1.0f, 0.0f, 0.0f);
        compIndex = 0;
    }
    else if (maxComp == tangentAbs.y)
    {
        refVector = float3(0.0f, 1.0f, 0.0f);
        compIndex = 1;
    }
    else
    {
        refVector = float3(0.0f, 0.0f, 1.0f);
        compIndex = 2;
    }
    
    float3 orthoA = normalize(cross(normal, refVector));
    float3 orthoB = cross(normal, orthoA);
    uint cosAngle = uint((dot(tangent, orthoA) * 0.5f + 0.5f) * 255.0f);
    uint tangentHandedness = (dot(tangent, orthoB) > 0.0001f) ? 1 : 0;
    
    result |= (cosAngle & 0xff) << 20;
    result |= (compIndex & 0x3) << 28;
    result |= (tangentHandedness & 0x1) << 30;
    result |= (binormalHandedness & 0x1) << 31;
    
    return result;
}

//-----------------------------------------------------------------------------
//      符号なしダイアモンドエンコードを行います.
//-----------------------------------------------------------------------------
float EncodeDiamond(float2 v)
{
    float m = abs(v.x) + abs(v.y);
    float x = v.x / m;
    float s = sign(v.x);
    return -s * 0.25f * x + 0.5f + s * 0.25f;
}

//-----------------------------------------------------------------------------
//      符号なしダイアモンドデコードを行います.
//-----------------------------------------------------------------------------
float2 DecodeDiamond(float v)
{
    float2 result;
    float s = sign(v - 0.5f);
    result.x = -s * 4.0f * v + 1.0f + s * 2.0f;
    result.y =  s * (1.0f - abs(v.x));
    return normalize(v);
}

//-----------------------------------------------------------------------------
//      接線ベクトルを符号なしダイアモンドエンコードします.
//-----------------------------------------------------------------------------
float EncodeTangent(float3 normal, float3 tangent)
{
    float3 t1;
    if (abs(normal.y) > abs(normal.z))
        t1 = float3(normal.y, -normal.x, 0.0f);
    else
        t1 = float3(normal.z, 0.0f, -normal.x);

    t1 = normalize(t1);
    float3 t2 = cross(t1, normal);
    float2 packedTangent = float2(dot(tangent, t1), dot(tangent, t2));
    return EncodeDiamond(packedTangent);
}

//-----------------------------------------------------------------------------
//      接線ベクトルを符号なしダイアモンドデコードします.
//-----------------------------------------------------------------------------
float3 DecodeTangent(float3 normal, float diamondTangent)
{
    float3 t1;
    if (abs(normal.y) > abs(normal.z))
        t1 = float3(normal.y, -normal.x, 0.0f);
    else
        t1 = float3(normal.z, 0.0f, -normal.x);
    
    t1 = normalize(t1);
    float3 t2 = cross(t1, normal);
    float2 packedTangent = DecodeDiamond(diamondTangent);
    return packedTangent.x * t1 + packedTangent.y * t2;
}

//-----------------------------------------------------------------------------
//      符号付きダイアモンドエンコードを行います.
//-----------------------------------------------------------------------------
void EncodeSignedDiamond(float2 v, out float diamond, out bool sign)
{
    float m = abs(v.x) + abs(v.y);
    float x = v.x / m;
    sign = (v.y >= 0.0f) ? true : false;
    diamond = x * 0.5f + 0.5f;
}

//-----------------------------------------------------------------------------
//      符号付きダイアモンドデコードを行います.
//-----------------------------------------------------------------------------
float2 DecodeSignedDiamond(float diamond, bool sign)
{
    float2 result;
    float s = sign ? 1.0f : -1.0f;
    result.x = 2.0f * diamond - 1.0f;
    result.y = s * (1.0f - abs(result.x));
    return normalize(result);
}

//-----------------------------------------------------------------------------
//      接線空間を計算します.
//-----------------------------------------------------------------------------
float3x3 CalcTangentFrame(float3 N, float3 p, float2 uv)
{
    // [Schüler2007] Christian Schüler,
    // “Normal Mapping without Precomputed Tangents”, 
    // ShaderX5, Chapter 2.6, pp.131-140, 2007.
    float3 dp1  = ddx(p);
    float3 dp2  = ddy(p);
    float2 duv1 = ddx(uv);
    float2 duv2 = ddy(uv);

    float2x3 M = float2x3(dp1, dp2);
    float3 T = mul(float2(duv1.x, duv2.x), M);
    float3 B = mul(float2(duv1.y, duv2.y), M);
    return float3x3(normalize(T), normalize(B), N);
}

//-----------------------------------------------------------------------------
//      余接空間を計算します.
//-----------------------------------------------------------------------------
float3x3 CalcCotangentFrame(float3 N, float3 p, float2 uv)
{
    // [Schüler2013] Christian Schüler,
    // “Followup: Normal Mapping Without Precomputed Tangents”,
    // http://www.thetenthplanet.de/archives/1180, 2013.
    float3 dp1  = ddx(p);
    float3 dp2  = ddy(p);
    float2 duv1 = ddx(uv);
    float2 duv2 = ddy(uv);

    float3 dp2perp = cross(dp2, N);
    float3 dp1perp = cross(N, dp1);
    float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    float3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    float invmax = rsqrt(max(dot(T, T), dot(B, B)));
    return float3x3(T * invmax, B * invmax, N);
}

//-----------------------------------------------------------------------------
//      接線をシフトします.
//-----------------------------------------------------------------------------
float3 ShiftTangent(float3 T, float3 N, float shiftAngle)
{
    // ShaderX 3の式ではなく，数学的に正しく回転を扱う式に変更.
    float cosTX = cos(shiftAngle);
    float sinTX = sqrt(1.0f - cosTX * cosTX);
    float cosTN = dot(T, N);
    float sinTN = sqrt(1.0f - cosTN * cosTN);
    float3 X = (cosTX * sinTN - cosTN * sinTX) * T + sinTN * sinTX * N;
    return normalize(X);
}

#endif//ASDX_TANGENT_SPACE_HLSLI
