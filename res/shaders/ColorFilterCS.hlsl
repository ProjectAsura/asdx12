//-----------------------------------------------------------------------------
// File : ColorFilterCS.hlsl
// Desc : Compute Shader for Color Filter.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Math.hlsli"

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
//      ���C���G���g���[�|�C���g�ł�.
//-----------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void main
(
    uint3 dispatchId : SV_DispatchThreadID,
    uint  groupIndex : SV_GroupIndex
)
{
    uint2 id = RemapLane8x8(dispatchId.xy, groupIndex);
    Output[id] = mul(ColorMatrix, Input[id]);
}
