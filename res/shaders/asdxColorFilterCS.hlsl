//-----------------------------------------------------------------------------
// File : asdxColorFilterCS.hlsl
// Desc : Compute Shader for Color Filter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxComputeUtil.hlsli"
#include "asdxSamplers.hlsli"


///////////////////////////////////////////////////////////////////////////////
// CbParam0 constant buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbParam0 : register(b0)
{
    float4x4    ColorMatrix;
};

///////////////////////////////////////////////////////////////////////////////
// CbParam1 constant buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbParam1 : register(b1)
{
    uint    DstResolution;
    uint3   Reserved;
};

//-----------------------------------------------------------------------------
// Resources.
//-----------------------------------------------------------------------------
Texture2D<float4>   Input   : register(t0);
RWTexture2D<float4> Output  : register(u0);

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void main
(
    uint3 dispatchId : SV_DispatchThreadID,
    uint  groupIndex : SV_GroupIndex
)
{
    uint2 remappedId = RemapLane8x8(dispatchId.xy, groupIndex);
    uint2 dstSize    = GetTargetSize(DstResolution);
    if (any(remappedId >= dstSize)) 
    { return; }

    float2 uv = remappedId / float2(dstSize);
    float4 texel = Input.SampleLevel(LinearClamp, uv, 0.0f);
    Output[remappedId] = mul(ColorMatrix, texel);
}

