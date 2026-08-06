//-----------------------------------------------------------------------------
// File : asdxStarCompositeCS.hlsl
// Desc : Star Composite.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxSamplers.hlsli"
#include "asdxComputeUtil.hlsli"


///////////////////////////////////////////////////////////////////////////////
// Param constant buffers.
///////////////////////////////////////////////////////////////////////////////
cbuffer Param : register(b0)
{
    uint  Resolution;
    uint3 Reserved;
};

//-----------------------------------------------------------------------------
// Resources.
//-----------------------------------------------------------------------------
Texture2D<float4>   InputMap  : register(t0);
RWTexture2D<float4> OutputMap : register(u0);

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex)
{
    uint2 remapId = RemapLane8x8(dispatchId.xy, groupIndex);

    uint2 targetSize = GetTargetSize(Resolution);
    if (any(remapId >= targetSize))
        return;

    // 縮小サイズの場合も考慮してサンプル命令でカラーを取得.
    float2 uv = (remapId + 0.5f.xx) / float2(targetSize);
    float4 color = 0.0f.xxxx;

    // 加算合成.
    color += InputMap.SampleLevel(LinearClamp, uv, 0.0f);
    color += OutputMap[remapId];

    // 更新.
    OutputMap[remapId] = color;
}
