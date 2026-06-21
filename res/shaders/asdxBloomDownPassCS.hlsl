//-----------------------------------------------------------------------------
// File : asdxBloomDownPassCS.hlsl
// Desc : Compute Shader For Bloom.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxComputeUtil.hlsli"
#include "asdxSamplers.hlsli"


//-----------------------------------------------------------------------------
// Resources
//-----------------------------------------------------------------------------
Texture2D<float4>   ColorMap    : register(t0);
RWTexture2D<float4> FilteredMap : register(u0);


///////////////////////////////////////////////////////////////////////////////
// CbParam constant buffers.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbParam : register(b0)
{
    uint    SrcResolution;
    uint    DstResolution;
    uint2   Reserved;
};

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
    uint2 remapId = RemapLane8x8(dispatchId.xy, groupIndex);
    uint dstW = DstResolution & 0xFFFF;
    uint dstH = (DstResolution >> 16) & 0xFFFF;

    if (any(remapId > uint2(dstW, dstH)))
        return;

    uint srcW = SrcResolution & 0xFFFF;
    uint srcH = (SrcResolution >> 16) & 0xFFFF;
 
    float2 invSrcSize = float2(1.0f / float(srcW), 1.0f / float(srcW));
    float2 uv = float2(remapId) * invSrcSize;   // ピクセル中心に移動させない.
    
    float4 result = 0.0f.xxxx;

    // 中心4テクセル.
    {
        float4 c0 = ColorMap.SampleLevel(LinearClamp, uv + float2(-1.0f, -1.0f) * invSrcSize, 0.0f);
        float4 c1 = ColorMap.SampleLevel(LinearClamp, uv + float2( 1.0f, -1.0f) * invSrcSize, 0.0f);
        float4 c2 = ColorMap.SampleLevel(LinearClamp, uv + float2(-1.0f,  1.0f) * invSrcSize, 0.0f);
        float4 c3 = ColorMap.SampleLevel(LinearClamp, uv + float2( 1.0f,  1.0f) * invSrcSize, 0.0f);

        float4 b0 = (c0 + c1 + c2 + c3) * 0.25f;
        result += b0 * 0.5f;
    }
 
    // 左上.
    {
        float4 c0 = ColorMap.SampleLevel(LinearClamp, uv + float2(-2.0f, -2.0f) * invSrcSize, 0.0f);
        float4 c1 = ColorMap.SampleLevel(LinearClamp, uv + float2( 0.0f, -2.0f) * invSrcSize, 0.0f);
        float4 c2 = ColorMap.SampleLevel(LinearClamp, uv + float2(-2.0f,  0.0f) * invSrcSize, 0.0f);
        float4 c3 = ColorMap.SampleLevel(LinearClamp, uv + float2( 0.0f,  0.0f) * invSrcSize, 0.0f);

        float4 b0 = (c0 + c1 + c2 + c3) * 0.25f;
        result += b0 * 0.125f;
    }
 
    // 右上
    {
        float4 c0 = ColorMap.SampleLevel(LinearClamp, uv + float2( 0.0f, -2.0f) * invSrcSize, 0.0f);
        float4 c1 = ColorMap.SampleLevel(LinearClamp, uv + float2( 2.0f, -2.0f) * invSrcSize, 0.0f);
        float4 c2 = ColorMap.SampleLevel(LinearClamp, uv + float2( 0.0f,  0.0f) * invSrcSize, 0.0f);
        float4 c3 = ColorMap.SampleLevel(LinearClamp, uv + float2( 2.0f,  0.0f) * invSrcSize, 0.0f);

        float4 b0 = (c0 + c1 + c2 + c3) * 0.25f;
        result += b0 * 0.125f;
    }

    // 左下
    {
        float4 c0 = ColorMap.SampleLevel(LinearClamp, uv + float2(-2.0f,  0.0f) * invSrcSize, 0.0f);
        float4 c1 = ColorMap.SampleLevel(LinearClamp, uv + float2( 0.0f,  0.0f) * invSrcSize, 0.0f);
        float4 c2 = ColorMap.SampleLevel(LinearClamp, uv + float2(-2.0f, -2.0f) * invSrcSize, 0.0f);
        float4 c3 = ColorMap.SampleLevel(LinearClamp, uv + float2( 0.0f, -2.0f) * invSrcSize, 0.0f);

        float4 b0 = (c0 + c1 + c2 + c3) * 0.25f;
        result += b0 * 0.125f;
    }

    // 右下
    {
        float4 c0 = ColorMap.SampleLevel(LinearClamp, uv + float2( 0.0f,  0.0f) * invSrcSize, 0.0f);
        float4 c1 = ColorMap.SampleLevel(LinearClamp, uv + float2( 2.0f,  0.0f) * invSrcSize, 0.0f);
        float4 c2 = ColorMap.SampleLevel(LinearClamp, uv + float2( 0.0f,  2.0f) * invSrcSize, 0.0f);
        float4 c3 = ColorMap.SampleLevel(LinearClamp, uv + float2( 2.0f,  2.0f) * invSrcSize, 0.0f);

        float4 b0 = (c0 + c1 + c2 + c3) * 0.25f;
        result += b0 * 0.125f;
    }

    // 結果を書き込み.
    FilteredMap[remapId] = result;
}
