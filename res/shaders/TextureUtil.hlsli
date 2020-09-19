//-----------------------------------------------------------------------------
// File : TextureUtil.hlsli
// Desc : Math Utility.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#ifndef ASDX_TEXTURE_UTIL_HLSLI
#define ASDX_TEXTURE_UTIL_HLSLI

//-----------------------------------------------------------------------------
//      �P�Ȃ鎋���}�b�s���O.
//-----------------------------------------------------------------------------
float2 ParallaxMapping
(
    Texture2D       heightMap,      // �����}�b�v.
    SamplerState    heightSmp,      // �T���v���[.
    float2          texcoord,       // �e�N�X�`�����W.
    float3          V,              // �����x�N�g��.
    float           heightScale     // �����X�P�[��.
)
{
    float  h = heightMap.Sample(heightSmp, texcoord).r;
    float2 p = (V.xy / V.z) * (h * heightScale);
    return texcoord - p;
}

//-----------------------------------------------------------------------------
//      �����Ւf�}�b�s���O.
//-----------------------------------------------------------------------------
float2 ParallxOcclusionMapping
(
    Texture2D           heightMap,      // �����}�b�v.
    SamplerState        heightSmp,      // �����}�b�v�p�T���v���[.
    Texture2D<float>    depthMap,       // �[�x�}�b�v.
    SamplerState        depthSmp,       // �[�x�}�b�v�p�T���v���[.
    float2              texcoord,       // �e�N�X�`�����W.
    float3              V,              // �����x�N�g��.
    float               heightScale,    // �����X�P�[��.
    const float         layerCount      // ���C���[��.
)
{
    float layerDepth     = 1.0f / layerCount; 
    float currLayerDepth = 0.0f;

    float2 p = V.xy * heightScale;
    float2 delta = p / layerCount;

    float2 currUV = texcoord;
    float currDepth = depthMap.Sample(depthSmp, currUV);

    [loop]
    while(currLayerDepth < currDepth)
    {
        currUV -= delta;
        currDepth = depthMap.Sample(depthSmp, currUV);
        currLayerDepth += layerDepth;
    }

    float2 prevUV = currUV + delta;
    float afterDepth = currDepth - currLayerDepth;
    float beforeDepth = depthMap.Sample(depthSmp, prevUV) - currLayerDepth + layerDepth;

    float weight = afterDepth / (afterDepth - beforeDepth);
    return lerp(currUV, prevUV, weight);
}

//-----------------------------------------------------------------------------
//      MinMip�}�b�v���T���v�����O���܂�.
//-----------------------------------------------------------------------------
float SampleMinLodFilter2D
(
    Texture2D       sampledTex,
    uint2           sampledTexDim,
    Texture2D       minlodTex,
    uint2           minlodTexDim
    float2          uv,
    SamplerState    smp,
    float4          filterSlope
)
{
    uint2 minLodAspect = sampledTexDim / minlodTexDim;

    int slopeU = filterSlope.x;
    int slopeV = filterSlope.y;
    if (minLodAspect.x > minLodAspect.y)
    { slopeU *= (minLodAspect.x / minLodAspect.y); }
    else
    { slopeV *= (minLodAspect.y / minLodAspect.x); }

    float2 absoluteCoords = float2(minLodTexDim) * uv;
    float2 clampedCoords = round(absoluteCoords) / float(minLodTexDim);
    float4 rawSamples = minlodtex.Gather(smp, clampedCoords).zwyx;

    float fracU = frac(absoluteCoords.x);
    float fracV = 1.0f - frac(absoluteCoord.y);

    float offsetU = filterSlopes.z;
    float offsetV = filterSlopes.w;

    int nearestTexel = ((fracU < 0.5f) ? 0 : 1) | ((fracV < 0.5f) ? 0 : 2);
    float gradientU = (fracU < 0.5f) ? fracU - offsetU : 1.0f - fracU - offsetU;
    float gradientV = (fracV < 0.5f) ? fracV - offsetV : 1.0f - fracV - offsetV;
    float weightU = saturate(gradientU * slopeU);
    float weightV = saturate(gradientV * slopeV);

    float4 newSamples;
    newSamples.x = rawSamples[nearestTexel];
    newSamples.y = max(rawSamples[nearestTexel & 2], rawSamples[(nearestTexel & 2) | 1]);
    newSamples.z = max(rawSamples[nearestTexel & 1], rawSamples[(nearestTexel & 1) | 2]);
    newSamples.w = max(rawSamples.x, max(rawSamples.y, max(rawSamples.z, rawSamples.w)));

    float revWeightU = 1.0f - weightU;
    float revWeightV = 1.0f - weightV;

    return newSamples.x *    weightU *    weightV
         + newSamples.y * revWeightU *    weightV
         + newSamples.z *    weightU * revWeightV
         + newSamples.w * revWeightU * revWeightV;
}

//-----------------------------------------------------------------------------
//      �ŏ��[�x��ۂ��Ȃ���[�x���_�E���T���v�����܂�.
//-----------------------------------------------------------------------------
float NearstDepthDownSample1(Texture2D<float> depthBuffer, SamplerState depthSampler, float2 uv)
{
    float4 depth = depthBuffer.GatherRed(depthSampler, uv);
    return min(depth.x, min(depth.y, min(depth.z, depth.w)));
}

//-----------------------------------------------------------------------------
//      �ŏ��[�x��ۂ��Ȃ���[�x���_�E���T���v�����܂�.
//-----------------------------------------------------------------------------
float NearstDepthDownSample4(Texture2D<float> depthBuffer, SamplerState depthSampler, float2 uv)
{
    float4 depth0 = depthBuffer.GatherRed(depthSampler, uv, int2(0, 0));
    float4 depth1 = depthBuffer.GatherRed(depthSampler, uv, int2(0, 2));
    float4 depth2 = depthBuffer.GatherRed(depthSampler, uv, int2(2, 0));
    float4 depth3 = depthBuffer.GatherRed(depthSampler, uv, int2(2, 2));

    float4 minDepth = min(depth0, min(depth1, min(depth2, depth3)));
    return min(minDepth.x, min(minDepth.y, min(minDepth.z, minDepth.w)));
}

//-----------------------------------------------------------------------------
//      �ő�[�x��ۂ��Ȃ���[�x���_�E���T���v�����܂�.
//-----------------------------------------------------------------------------
float FarestDepthDownSample1(Texture2D<float> depthBuffer, SamplerState depthSampler, float2 uv)
{
    float4 depth = depthBuffer.GatherRed(depthSampler, uv);
    return max(depth.x, max(depth.y, max(depth.z, depth.w)));
}

//-----------------------------------------------------------------------------
//      �ő�[�x��ۂ��Ȃ���[�x���_�E���T���v�����܂�.
//-----------------------------------------------------------------------------
float FarestDepthDownSample4(Texture2D<float> depthBuffer, SamplerState depthSampler, float2 uv)
{
    float4 depth0 = depthBuffer.GatherRed(depthSampler, uv, int2(0, 0));
    float4 depth1 = depthBuffer.GatherRed(depthSampler, uv, int2(0, 2));
    float4 depth2 = depthBuffer.GatherRed(depthSampler, uv, int2(2, 0));
    float4 depth3 = depthBuffer.GatherRed(depthSampler, uv, int2(2, 2));

    float4 maxDepth = max(depth0, max(depth1, max(depth2, depth3)));
    return max(maxDepth.x, max(maxDepth.y, max(maxDepth.z, maxDepth.w)));
}

#endif//ASDX_TEXTURE_UTIL_HLSLI
