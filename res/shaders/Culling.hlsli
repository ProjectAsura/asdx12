//-----------------------------------------------------------------------------
// File : Culling.hlsli
// Desc : Culling Utility Functions.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#ifndef ASDX_CULLING_HLSLI
#define ASDX_CULLING_HLSLI


//-----------------------------------------------------------------------------
//      法錐カリングを行います.
//-----------------------------------------------------------------------------
bool NormalConeCulling(float4 normalCone, float3 viewDir)
{
    // normalConeはワールド変換済みとします.
    return dot(normalCone.xyz, -viewDir) > normalCone.w;
}

//-----------------------------------------------------------------------------
//      視錐台の中にスフィアが含まれるかどうかチェックします.
//-----------------------------------------------------------------------------
bool Contains(float4 planes[6], float4 sphere)
{
    // sphereは事前に位置座標がワールド変換済み，半径もスケール適用済みとします.
    float4 center = float4(sphere.xyz, 1.0f);

    for (int i = 0; i < 6; ++i)
    {
        if (dot(center, planes[i]) < -sphere.w)
        {
            // カリングする.
            return false;
        }
    }

    // カリングしない.
    return true;
}

//-----------------------------------------------------------------------------
//      背面カリングとゼロ面積カリング.
//-----------------------------------------------------------------------------
bool IsBackFaceOrZeroArea(float3 worldPos[3], float3 cameraPos)
{
    float3 viewDir = normalize(cameraPos - worldPos[0]);

    float3 a = worldPos[1].xyz - worldPos[0].xyz;
    float3 b = worldPos[2].xyz - worldPos[0].xyz;
    float3 n = cross(a, b);
    return dot(n, viewDir) >= 0.0f; // カリングする.
}

//-----------------------------------------------------------------------------
//      視錐台カリングと微小プリミティブカリング.
//-----------------------------------------------------------------------------
bool PrimitiveCulling(float2 posSS[3], float2 renderTargetSize)
{
    bool culled = false;

    float2 mini = 1.0f.xx;
    float2 maxi = 0.0f.xx;

    // 視錐台カリング.
    for (uint i = 0; i < 3; ++i)
    {
        mini = min(mini, posSS[i]);
        maxi = max(maxi, posSS[i]);
    }
    culled |= (any(mini > 1.0f) || any(maxi < 0.0f)); // カリングする.

    // 微小プリミティブカリング.
    maxi *= renderTargetSize;
    mini *= renderTargetSize;
    culled |= any(round(mini) == round(maxi)); // カリングする.

    return culled;
}

//-----------------------------------------------------------------------------
//      スクリーン上の矩形を求めます.
//-----------------------------------------------------------------------------
float4 SphereScreenExtents(float4 sphere, float4x4 viewProj)
{
    // https://gist.github.com/JarkkoPFC/1186bc8a861dae3c8339b0cda4e6cdb3
    float4 result;
    float r2 = sphere.w * sphere.w;
    float d = sphere.z * sphere.w;

    float hv = sqrt(sphere.x * sphere.x + sphere.z * sphere.z - r2);
    float ha = sphere.x * hv;
    float hb = sphere.x * sphere.w;
    float hc = sphere.z * hv;
    result.x = (ha - d) * viewProj._11 / (hc + hb); // left
    result.z = (ha + d) * viewProj._11 / (hc - hb); // right

    float vv = sqrt(sphere.y * sphere.y + sphere.z * sphere.z - r2);
    float va = sphere.y * vv;
    float vb = sphere.y * sphere.w;
    float vc = sphere.z * vv;
    result.y = (va - d) * viewProj._22 / (vc + vb); // bottom
    result.w = (va + d) * viewProj._22 / (vc - vb); // top.

    return result;
}

//-----------------------------------------------------------------------------
//      寄与カリングを行います.
//-----------------------------------------------------------------------------
bool ContributionCulling(float4 sphere, float4x4 viewProj, float minContribution)
{
    float4 LBRT = SphereScreenExtents(sphere, viewProj);

    float w = abs(LBRT.z - LBRT.x); // (left - right).
    float h = abs(LBRT.w - LBRT.y); // (top - bottom).
    
    return max(w, h) < minContribution; // カリングする.
}

//-----------------------------------------------------------------------------
//      誤差を投影します.
//-----------------------------------------------------------------------------
float ProjectError(float3 center, float radius, float screenScaleY)
{
    // https://stackoverflow.com/questions/21648630/radius-of-projected-sphere-in-screen-space
    if (isinf(radius))
        return radius;

    // screenScaleY = height * 0.5f * (1.0f / tan(fov * 0.5f)) とします.
    float d2 = dot(center, center);
    return screenScaleY * radius / sqrt(d2 - radius * radius);
}

//-----------------------------------------------------------------------------
//      30-bitにパッキングされたプリミティブ番号を展開します.
//-----------------------------------------------------------------------------
uint3 UnpackPrimitiveIndex(uint packed)
{
    return uint3(
        packed & 0x3FF,
        (packed >> 10) & 0x3FF,
        (packed >> 20) & 0x3FF);
}

//-----------------------------------------------------------------------------
//      8-bit プリミティブインデックスを取得します.
//-----------------------------------------------------------------------------
uint3 GetPrimitiveIndex(ByteAddressBuffer indexBuffer, uint triangleIndex)
{
    // 3バイト単位で三角形のインデックスが格納されている
    uint baseByteOffset = triangleIndex * 3;

    // 各インデックスのバイトオフセット
    uint3 byteOffset = uint3(baseByteOffset + 0, baseByteOffset + 1, baseByteOffset + 2);

    // 各インデックスが含まれる4バイト境界を計算
    uint3 alignedOffset = byteOffset & (~3u).xxx;

    // バイトをまたぐ場合を考慮して，8バイトロード.
    uint2 raw = indexBuffer.Load2(alignedOffset.x);

    // 必要なデータを決定.
    uint3 data;
    data.x = raw.x;
    data.y = (alignedOffset.y != alignedOffset.x) ? raw.y : data.x;
    data.z = (alignedOffset.z != alignedOffset.y) ? raw.y : data.y;

    // シフト量
    uint3 shift = (byteOffset & (3u).xxx) * 8;

    // 抽出（それぞれの正しいロード結果から抽出）
    return (data >> shift) & 0xFF.xxx;
}

#endif//ASDX_CULLING_HLSLI
