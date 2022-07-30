//-----------------------------------------------------------------------------
// File : ColorFilterCS.hlsl
// Desc : Compute Shader for Color Filter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Math.hlsli"

// スレッドサイズ.
#define THREAD_SIZE 8

//-----------------------------------------------------------------------------
// Constant Values.
//-----------------------------------------------------------------------------
static const uint2 kThreadSize = uint2(THREAD_SIZE, THREAD_SIZE);


///////////////////////////////////////////////////////////////////////////////
// CbColorFilter constant buffer.
///////////////////////////////////////////////////////////////////////////////
cbuffer CbColorFilter : register(b0)
{
    uint2       DispatchDim;
    float2      InvTargetSize;
    float4x4    ColorMatrix;
};

//-----------------------------------------------------------------------------
// Resources.
//-----------------------------------------------------------------------------
Texture2D<float4>   Input   : register(t0);
RWTexture2D<float4> Output  : register(u0);

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
[numthreads(THREAD_SIZE, THREAD_SIZE, 1)]
void main
(
    uint3 groupId       : SV_GroupID,
    uint3 groupThreadId : SV_GroupThreadID
)
{
    uint2 id = RemapThreadId(kThreadSize, DispatchDim, 16, groupId.xy, groupThreadId.xy);
    Output[id] = mul(ColorMatrix, Input[id]);
}
