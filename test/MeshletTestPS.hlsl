//-----------------------------------------------------------------------------
// File : MeshletTestPS.hlsl
// Desc : Test For Meshlet Draw.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "asdxMath.hlsli"


///////////////////////////////////////////////////////////////////////////////
// MSOutput structure
///////////////////////////////////////////////////////////////////////////////
struct MSOutput
{
    float4  Position    : SV_POSITION;
    float2  TexCoord    : TEXCOORD;
    float3  Normal      : NORMAL;
    float3  Tangent     : TANGENT;
};

//-----------------------------------------------------------------------------
//      デバッグカラーを取得します.
//-----------------------------------------------------------------------------
float3 DebugColor(uint index)
{
    return float3(
        float(index & 1),
        float(index & 3) / 4,
        float(index & 7) / 8);
}

//-----------------------------------------------------------------------------
//      メインエントリーポイントです.
//-----------------------------------------------------------------------------
float4 main(const MSOutput input) : SV_TARGET
{
    //return float4(DebugColor(input.TexCoord.z), 
    //return float4(input.TexCoord, 0.0f, 1.0f);
    return float4(input.Tangent * 0.5f + 0.5f, 1.0f);
    //return float4(input.Normal * 0.5f + 0.5f, 1.0f);
}