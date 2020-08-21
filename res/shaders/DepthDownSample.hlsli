//-----------------------------------------------------------------------------
// File : DepthDownSample.hlsli
// Desc : Depth Down Sampling.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------
#ifndef ASDX_DEPTH_DOWN_SAMPLE_HLSLI
#define ASDX_DEPTH_DOWN_SAMPLE_HLSLI

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Math.hlsli"

//-----------------------------------------------------------------------------
//      深度をダウンサンプルします.
//-----------------------------------------------------------------------------
float DepthDownSample(Texture2D<float> depthBuffer, SamplerState depthSampler, float2 uv)
{
    float4 depth0 = depthBuffer.GatherRed(depthSampler, uv, int2(0, 0));
    float4 depth1 = depthBuffer.GatherRed(depthSampler, uv, int2(0, 2));
    float4 depth2 = depthBuffer.GatherRed(depthSampler, uv, int2(2, 0));
    float4 depth3 = depthBuffer.GatherRed(depthSampler, uv, int2(2, 2));

    float4 minDepth = min(depth0, min(depth1, min(depth2, depth3)));
    return Min4(minDepth);
}

#endif//ASDX_DEPTH_DOWN_SAMPLE_HLSLI